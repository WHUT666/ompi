/*
 * Copyright (c) 2026      Amazon.com, Inc. or its affiliates.
 *                         All Rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 * SPDX-License-Identifier: BSD-3-Clause-Open-MPI
 */
/* POSIX asynchronous I/O for the native Windows port.
 *
 * aio_read/aio_write spawn a worker thread that performs *positioned*
 * synchronous I/O on the fd's own handle (an OVERLAPPED passed to a
 * synchronous file handle supplies the byte offset and leaves the file
 * pointer alone).  Using the fd handle itself -- rather than a
 * ReOpenFile'd twin -- means byte-range locks taken through fcntl()
 * admit the async ops: Windows enforces such locks per file object,
 * while POSIX fcntl locks only exclude *other* processes.
 * Completion is tracked with manual-reset events: aio_error() polls,
 * aio_suspend() waits on the events, and aio_return() reaps.
 */
#ifndef OPAL_WIN32_AIO_H
#define OPAL_WIN32_AIO_H

#ifndef _WIN32
#    error "opal/win32 aio shim is Windows-only"
#endif

#include "opal_win32_common.h"
#include <sys/types.h>
#include <time.h>
#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif

/* sigev_notify values (only SIGEV_NONE is meaningful for fbtl) */
#define SIGEV_NONE     0
#define SIGEV_SIGNAL   1
#define SIGEV_THREAD   2

/* aio_cancel return values */
#define AIO_CANCELED    0
#define AIO_NOTCANCELED 1
#define AIO_ALLDONE     2

/* lio_listio modes/opcodes */
#define LIO_WAIT    0
#define LIO_NOWAIT  1
#define LIO_READ    0
#define LIO_WRITE   1
#define LIO_NOP     2

struct sigevent {
    int     sigev_notify;
    int     sigev_signo;
    union {
        int   sival_int;
        void *sival_ptr;
    }       sigev_value;
    void  (*sigev_notify_function)(void *);
    void   *sigev_notify_attributes;
    int     sigev_notify_thread_id;
};

struct aiocb {
    int              aio_fildes;
    off_t            aio_offset;
    volatile void   *aio_buf;
    size_t           aio_nbytes;
    int              aio_reqprio;
    struct sigevent  aio_sigevent;
    int              aio_lio_opcode;

    /* Windows bookkeeping (opaque to callers): _aio_ov carries the
     * per-op completion event, _aio_worker the I/O thread handle. */
    OVERLAPPED       _aio_ov;
    HANDLE           _aio_worker;    /* worker thread handle */
    volatile LONG    _aio_state;   /* 0 idle, 1 pending, 2 done */
    DWORD            _aio_xfer;    /* bytes transferred */
    DWORD            _aio_error;   /* Win32 error code of completed op */
};

OPAL_WIN32_DECLSPEC int     aio_read(struct aiocb *cb);
OPAL_WIN32_DECLSPEC int     aio_write(struct aiocb *cb);
OPAL_WIN32_DECLSPEC int     aio_error(const struct aiocb *cb);
OPAL_WIN32_DECLSPEC ssize_t aio_return(struct aiocb *cb);
OPAL_WIN32_DECLSPEC int     aio_suspend(const struct aiocb *const list[], int n,
                                        const struct timespec *timeout);
OPAL_WIN32_DECLSPEC int     aio_cancel(int fd, struct aiocb *cb);
OPAL_WIN32_DECLSPEC int     aio_fsync(int op, struct aiocb *cb);
OPAL_WIN32_DECLSPEC int     lio_listio(int mode, struct aiocb *const list[], int n,
                                       struct sigevent *sig);

#ifdef __cplusplus
}
#endif

#endif /* OPAL_WIN32_AIO_H */
