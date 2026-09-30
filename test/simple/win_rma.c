/* -*- Mode: C; c-basic-offset:4 ; indent-tabs-mode:nil -*- */
/*
 * Copyright (c) 2026      Amazon.com, Inc. or its affiliates.
 *                         All Rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 *
 * One-sided (RMA) coverage for the Windows port.
 *
 *  - MPI_Win_create + fence + Put/Get      (osc/rdma over btl/tcp)
 *  - MPI_Win_allocate + Put/Get            (osc/sm shared segment)
 *  - MPI_Win_allocate_shared + shared_query+ direct load/store
 *  - MPI_Win_lock/unlock + Win_flush       (osc/sm passive target)
 *  - blocking_fence info key               (process-shared mutex+cond
 *                                           inside the shared segment)
 *
 * Expected to be run with "mpiexec -n 2".
 */
#include <mpi.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static int rank, size, fails = 0;

#define TCHK(name, expr)                                                          \
    do {                                                                          \
        int _rc = (expr);                                                         \
        if (_rc != MPI_SUCCESS) {                                                 \
            char _s[256];                                                         \
            int _l = 0;                                                           \
            MPI_Error_string(_rc, _s, &_l);                                       \
            printf("[%d] FAIL %s rc=%d (%s)\n", rank, name, _rc, _s);             \
            fflush(stdout);                                                       \
            fails++;                                                              \
        } else {                                                                  \
            printf("[%d] PASS %s\n", rank, name);                                 \
            fflush(stdout);                                                       \
        }                                                                         \
    } while (0)

static void check_int(const char *name, int got, int want)
{
    if (got == want) {
        printf("[%d] PASS %s val=%d\n", rank, name, got);
    } else {
        printf("[%d] FAIL %s got %d want %d\n", rank, name, got, want);
        fails++;
    }
    fflush(stdout);
}

/* classic fence-based put/get on user-provided memory */
static void test_win_create(void)
{
    int *winbuf = calloc((size_t) size, sizeof(int));
    MPI_Win win;

    TCHK("Win_create", MPI_Win_create(winbuf, sizeof(int) * size, sizeof(int),
                                      MPI_INFO_NULL, MPI_COMM_WORLD, &win));
    if (0 != fails) {
        free(winbuf);
        return;
    }

    MPI_Win_fence(0, win);
    int val = 100 + rank;
    TCHK("Win_put", MPI_Put(&val, 1, MPI_INT, 0, rank, 1, MPI_INT, win));
    MPI_Win_fence(0, win);
    if (0 == rank) {
        int ok = 1;
        for (int i = 0; i < size; i++) {
            if (winbuf[i] != 100 + i) {
                ok = 0;
            }
        }
        if (!ok) {
            printf("[%d] FAIL Win_put visible\n", rank);
            fails++;
        } else {
            printf("[%d] PASS Win_put visible\n", rank);
        }
        fflush(stdout);
    }

    MPI_Win_fence(0, win);
    int got = -1;
    TCHK("Win_get", MPI_Get(&got, 1, MPI_INT, 0, rank, 1, MPI_INT, win));
    MPI_Win_fence(0, win);
    check_int("Win_get", got, 100 + rank);
    TCHK("Win_free", MPI_Win_free(&win));
    free(winbuf);
}

/* MPI-3 allocate window: local ranks should run through osc/sm */
static void test_win_allocate(void)
{
    int *winbuf = NULL;
    MPI_Win win;

    TCHK("Win_allocate",
         MPI_Win_allocate(sizeof(int) * size, sizeof(int), MPI_INFO_NULL,
                          MPI_COMM_WORLD, &winbuf, &win));
    if (NULL == winbuf) {
        printf("[%d] FAIL Win_allocate returned NULL base\n", rank);
        fails++;
        return;
    }
    for (int i = 0; i < size; i++) {
        winbuf[i] = -1;
    }

    MPI_Win_fence(0, win);
    int val = 200 + rank;
    TCHK("alloc Win_put", MPI_Put(&val, 1, MPI_INT, 0, rank, 1, MPI_INT, win));
    MPI_Win_fence(0, win);

    MPI_Win_fence(0, win);
    int got = -1;
    TCHK("alloc Win_get", MPI_Get(&got, 1, MPI_INT, 0, rank, 1, MPI_INT, win));
    MPI_Win_fence(0, win);
    check_int("alloc Win_get", got, 200 + rank);

    /* passive-target lock/unlock + flush against rank 0 */
    TCHK("Win_lock", MPI_Win_lock(MPI_LOCK_EXCLUSIVE, 0, 0, win));
    val = 300 + rank;
    TCHK("lock Win_put", MPI_Put(&val, 1, MPI_INT, 0, rank, 1, MPI_INT, win));
    TCHK("Win_flush", MPI_Win_flush(0, win));
    TCHK("Win_unlock", MPI_Win_unlock(0, win));
    MPI_Barrier(MPI_COMM_WORLD);
    if (0 == rank) {
        int ok = 1;
        for (int i = 0; i < size; i++) {
            if (winbuf[i] != 300 + i) {
                ok = 0;
            }
        }
        if (!ok) {
            printf("[%d] FAIL lock put visible\n", rank);
            fails++;
        } else {
            printf("[%d] PASS lock put visible\n", rank);
        }
        fflush(stdout);
    }

    TCHK("Win_free(2)", MPI_Win_free(&win));
}

/* shared-flavor window: direct load/store into the shared segment */
static void test_win_allocate_shared(void)
{
    int *base = NULL;
    MPI_Win win;

    TCHK("Win_allocate_shared",
         MPI_Win_allocate_shared(sizeof(int) * size, sizeof(int), MPI_INFO_NULL,
                                 MPI_COMM_WORLD, &base, &win));
    if (NULL == base) {
        printf("[%d] FAIL Win_allocate_shared returned NULL base\n", rank);
        fails++;
        return;
    }

    for (int i = 0; i < size; i++) {
        base[i] = -1;
    }
    MPI_Barrier(MPI_COMM_WORLD);

    /* direct store into my own slot of every peer's segment */
    for (int p = 0; p < size; p++) {
        int *peer_base = NULL;
        MPI_Aint psz = 0;
        int pdisp = 0;
        TCHK("Win_shared_query",
             MPI_Win_shared_query(win, p, &psz, &pdisp, &peer_base));
        if (NULL != peer_base && (size_t) psz >= sizeof(int) * size) {
            peer_base[rank] = 400 + rank;
        } else {
            printf("[%d] FAIL shared_query peer %d base=%p size=%ld\n", rank, p,
                   (void *) peer_base, (long) psz);
            fails++;
        }
    }
    MPI_Barrier(MPI_COMM_WORLD);

    int ok = 1;
    for (int i = 0; i < size; i++) {
        if (base[i] != 400 + i) {
            ok = 0;
        }
    }
    if (!ok) {
        printf("[%d] FAIL shared store visible\n", rank);
        fails++;
    } else {
        printf("[%d] PASS shared store visible\n", rank);
    }
    fflush(stdout);

    /* RMA get on the shared window must agree with direct load */
    int got = -1;
    TCHK("shared Win_get", MPI_Get(&got, 1, MPI_INT, 0, rank, 1, MPI_INT, win));
    MPI_Win_sync(win);
    MPI_Barrier(MPI_COMM_WORLD);
    check_int("shared Win_get", got, 400 + rank);

    TCHK("Win_free(3)", MPI_Win_free(&win));
}

/* Win_create with blocking_fence info: exercises the process-shared
 * mutex+cond inside the window's shared segment (osc_sm fence). */
static void test_blocking_fence(void)
{
    MPI_Info info;
    MPI_Info_create(&info);
    MPI_Info_set(info, "blocking_fence", "true");

    int *winbuf = NULL;
    MPI_Win win;
    TCHK("Win_create(blocking_fence)",
         MPI_Win_allocate(sizeof(int) * size, sizeof(int), info,
                          MPI_COMM_WORLD, &winbuf, &win));
    if (NULL == winbuf) {
        printf("[%d] FAIL blocking-fence window alloc\n", rank);
        fails++;
        MPI_Info_free(&info);
        return;
    }
    winbuf[rank] = -1;

    /* several fences to make the shared mutex/cond gate actually cycle */
    for (int r = 0; r < 4; r++) {
        MPI_Win_fence(0, win);
        int val = 500 + rank + r;
        MPI_Put(&val, 1, MPI_INT, 0, rank, 1, MPI_INT, win);
        MPI_Win_fence(0, win);
    }
    if (0 == rank) {
        int ok = 1;
        for (int i = 0; i < size; i++) {
            if (winbuf[i] != 500 + i + 3) {
                ok = 0;
            }
        }
        if (!ok) {
            printf("[%d] FAIL blocking fence visible\n", rank);
            fails++;
        } else {
            printf("[%d] PASS blocking fence visible\n", rank);
        }
        fflush(stdout);
    }
    TCHK("Win_free(4)", MPI_Win_free(&win));
    MPI_Info_free(&info);
}

int main(int argc, char **argv)
{
    setbuf(stdout, NULL);
    setbuf(stderr, NULL);
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    test_win_create();
    MPI_Barrier(MPI_COMM_WORLD);
    test_win_allocate();
    MPI_Barrier(MPI_COMM_WORLD);
    test_win_allocate_shared();
    MPI_Barrier(MPI_COMM_WORLD);
    test_blocking_fence();

    MPI_Barrier(MPI_COMM_WORLD);
    if (0 == rank) {
        printf("[%s] fails=%d\n", fails ? "RESULT FAIL" : "RESULT OK", fails);
        fflush(stdout);
    }
    MPI_Finalize();
    return fails;
}
