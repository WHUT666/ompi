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
 * Nonblocking MPI-IO coverage: MPI_File_iwrite_at / iread_at /
 * iread_at_all with MPI_Test + MPI_Wait.  On Windows this exercises
 * the aio_* emulation over kernel overlapped I/O in
 * opal/win32/opal_win32_aio.c (fbtl/posix AIO path).
 *
 * Large transfers are used so that the operations are realistically
 * asynchronous; completion semantics (Test/Wait, data integrity) are
 * what is asserted -- not that Test returns flag=0.
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

#define NB_COUNT (128 * 1024)   /* ints per rank: 512 KiB */

int main(int argc, char **argv)
{
    setbuf(stdout, NULL);
    setbuf(stderr, NULL);
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int *wbuf = malloc(NB_COUNT * sizeof(int));
    int *rbuf = malloc(NB_COUNT * sizeof(int));
    if (NULL == wbuf || NULL == rbuf) {
        printf("[%d] FAIL malloc\n", rank);
        MPI_Abort(MPI_COMM_WORLD, 1);
    }
    for (int i = 0; i < NB_COUNT; i++) {
        wbuf[i] = rank * NB_COUNT + i;
        rbuf[i] = -1;
    }

    MPI_File fh;
    const char *fname = "mpi_io_nb_test.dat";
    TCHK("File_open", MPI_File_open(MPI_COMM_WORLD, (char *) fname,
                                    MPI_MODE_CREATE | MPI_MODE_RDWR | MPI_MODE_DELETE_ON_CLOSE,
                                    MPI_INFO_NULL, &fh));
    if (0 != fails) {
        MPI_Finalize();
        return fails;
    }

    /* nonblocking write at explicit offset */
    MPI_Request wreq = MPI_REQUEST_NULL;
    MPI_Offset off = (MPI_Offset) rank * (MPI_Offset) (NB_COUNT * (MPI_Offset) sizeof(int));
    TCHK("File_iwrite_at",
         MPI_File_iwrite_at(fh, off, wbuf, NB_COUNT, MPI_INT, &wreq));

    /* exercise MPI_Test: poll until completion (flag may already be 1) */
    int flag = 0, polls = 0;
    MPI_Status st;
    while (!flag && polls < 1000000) {
        MPI_Test(&wreq, &flag, &st);
        polls++;
    }
    if (flag) {
        printf("[%d] PASS write completed after %d test polls\n", rank, polls);
    } else {
        printf("[%d] FAIL write never completed\n", rank);
        fails++;
    }

    TCHK("File_sync", MPI_File_sync(fh));
    MPI_Barrier(MPI_COMM_WORLD);

    /* nonblocking read of slot 0 (written by rank 0 above) */
    MPI_Request rreq = MPI_REQUEST_NULL;
    TCHK("File_iread_at",
         MPI_File_iread_at(fh, 0, rbuf, NB_COUNT, MPI_INT, &rreq));

    /* poll Test a bounded number of times, then block in Wait --
     * covers both the pending and already-complete paths */
    flag = 0;
    for (polls = 0; polls < 64 && !flag; polls++) {
        MPI_Test(&rreq, &flag, &st);
    }
    if (!flag) {
        TCHK("Wait(iread)", MPI_Wait(&rreq, &st));
    } else {
        printf("[%d] PASS iread already complete after %d polls\n", rank, polls);
    }

    int ok = 1;
    for (int i = 0; i < NB_COUNT; i++) {
        if (rbuf[i] != i) {
            ok = 0;
            break;
        }
    }
    if (ok) {
        printf("[%d] PASS iread data intact\n", rank);
    } else {
        printf("[%d] FAIL iread data mismatch\n", rank);
        fails++;
    }
    fflush(stdout);

    /* split-collective nonblocking read: everyone reads slot 0 again */
    MPI_Request creq = MPI_REQUEST_NULL;
    memset(rbuf, 0, NB_COUNT * sizeof(int));
    TCHK("File_iread_at_all",
         MPI_File_iread_at_all(fh, 0, rbuf, NB_COUNT, MPI_INT, &creq));
    TCHK("Wait(iread_all)", MPI_Wait(&creq, &st));
    ok = 1;
    for (int i = 0; i < NB_COUNT; i++) {
        if (rbuf[i] != i) {
            ok = 0;
            break;
        }
    }
    if (ok) {
        printf("[%d] PASS iread_at_all data intact\n", rank);
    } else {
        printf("[%d] FAIL iread_at_all data mismatch\n", rank);
        fails++;
    }
    fflush(stdout);

    TCHK("File_close", MPI_File_close(&fh));

    MPI_Barrier(MPI_COMM_WORLD);
    if (0 == rank) {
        printf("[%s] fails=%d\n", fails ? "RESULT FAIL" : "RESULT OK", fails);
        fflush(stdout);
    }
    MPI_Finalize();
    free(wbuf);
    free(rbuf);
    return fails;
}
