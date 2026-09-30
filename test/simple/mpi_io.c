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
 * Blocking MPI-IO coverage (OMPIO: fs/ufs + fbtl/posix + fcoll +
 * sharedfp/sm): open, explicit-offset write/read, file sync,
 * collective read, shared file pointer, close.
 *
 * Expected to be run with "mpiexec -n 2".
 */
#include <mpi.h>
#include <stdio.h>
#include <string.h>

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

int main(int argc, char **argv)
{
    setbuf(stdout, NULL);
    setbuf(stderr, NULL);
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    MPI_File fh;
    const char *fname = "mpi_io_test.dat";

    TCHK("File_open", MPI_File_open(MPI_COMM_WORLD, (char *) fname,
                                    MPI_MODE_CREATE | MPI_MODE_RDWR | MPI_MODE_DELETE_ON_CLOSE,
                                    MPI_INFO_NULL, &fh));
    if (0 != fails) {
        MPI_Finalize();
        return fails;
    }

    /* explicit-offset write: each rank owns slot[rank] */
    int wv = 1000 + rank;
    MPI_Offset off = (MPI_Offset) rank * (MPI_Offset) sizeof(int);
    TCHK("File_write_at",
         MPI_File_write_at(fh, off, &wv, 1, MPI_INT, MPI_STATUS_IGNORE));
    TCHK("File_sync", MPI_File_sync(fh));
    MPI_Barrier(MPI_COMM_WORLD);

    /* read own slot back */
    int rv = -1;
    TCHK("File_read_at",
         MPI_File_read_at(fh, off, &rv, 1, MPI_INT, MPI_STATUS_IGNORE));
    if (rv == wv) {
        printf("[%d] PASS File data val=%d\n", rank, rv);
    } else {
        printf("[%d] FAIL File data got %d want %d\n", rank, rv, wv);
        fails++;
    }
    fflush(stdout);

    /* collective read of slot 0 (written by rank 0) */
    rv = -1;
    TCHK("File_read_at_all",
         MPI_File_read_at_all(fh, 0, &rv, 1, MPI_INT, MPI_STATUS_IGNORE));
    if (rv == 1000) {
        printf("[%d] PASS collective read val=%d\n", rank, rv);
    } else {
        printf("[%d] FAIL collective read got %d want 1000\n", rank, rv);
        fails++;
    }
    fflush(stdout);

    /* shared file pointer path (sharedfp component).  The shared
     * pointer starts at 0 and each write_shared advances it; after
     * both ranks wrote, rewind to 0 and read sequentially -- every
     * read returns one of the shared-pointer writers' values. */
    int sv = 2000 + rank;
    TCHK("File_write_shared",
         MPI_File_write_shared(fh, &sv, 1, MPI_INT, MPI_STATUS_IGNORE));
    TCHK("File_sync(2)", MPI_File_sync(fh));
    MPI_Barrier(MPI_COMM_WORLD);

    TCHK("File_seek_shared", MPI_File_seek_shared(fh, 0, MPI_SEEK_SET));
    rv = -1;
    TCHK("File_read_shared",
         MPI_File_read_shared(fh, &rv, 1, MPI_INT, MPI_STATUS_IGNORE));
    if (rv >= 2000 && rv < 2000 + size) {
        printf("[%d] PASS shared-ptr read val=%d\n", rank, rv);
    } else {
        printf("[%d] FAIL shared-ptr read got %d\n", rank, rv);
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
    return fails;
}
