/*
 * Copyright (c) 2026      Amazon.com, Inc. or its affiliates.
 *                         All Rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 *
 * POSIX AIO emulation for the native Windows port.
 *
 * Modeled after the glibc approach: each aiocb gets a worker thread
 * that performs *positioned* synchronous I/O on the descriptor's own
 * handle.  A synchronous file HANDLE still honors the byte offset in a
 * supplied OVERLAPPED (it just never queues the operation), which gives
 * us positioned read/write without disturbing the shared file pointer.
 *
 * Two properties fall out of using the fd's handle in a worker:
 *   - real background parallelism => genuinely asynchronous File_i*,
 *   - byte-range locks taken through fcntl()/LockFileEx admit our I/O,
 *     because the lock is owned by the same file object -- matching
 *     POSIX, where fcntl locks only exclude *other* processes.
 *
 * Completion is tracked with a manual-reset event per op: aio_error()
 * polls it, aio_suspend() waits on the set, aio_return() reaps and
 * releases the worker resources.
 *
 * Mapped errno values: pending -> EINPROGRESS, canceled -> ECANCELED,
 * other errors -> EIO.  aio_return() yields the byte count like POSIX.
 */

#include "opal_config.h"

#ifdef _WIN32

#    include "opal/win32/opal_win32.h"
#    include <aio.h>

#    define AIO_IDLE     0
#    define AIO_PENDING  1
#    define AIO_DONE     2

typedef struct aio_work_t {
    struct aiocb *cb;
    HANDLE        fh;    /* fd file handle (not owned) */
    BOOL          write;
} aio_work_t;

/* Positioned read/write on a synchronous handle: OVERLAPPED supplies
 * the byte offset; the call completes before returning, and the file
 * pointer is left alone -- safe to run from several workers on the same
 * handle concurrently. */
static DWORD aio_do_io(aio_work_t *w)
{
    struct aiocb *cb = w->cb;
    OVERLAPPED ov;
    DWORD total = 0;

    while (total < cb->aio_nbytes) {
        DWORD step;
        BOOL ok;
        LONGLONG pos = cb->aio_offset + total;
        DWORD want;

        memset(&ov, 0, sizeof(ov));
        ov.Offset = (DWORD) (pos & 0xFFFFFFFF);
        ov.OffsetHigh = (DWORD) (((unsigned long long) pos) >> 32);
        want = (cb->aio_nbytes - total > 0x40000000ULL)
                   ? 0x40000000U
                   : (DWORD) (cb->aio_nbytes - total);
        if (w->write) {
            ok = WriteFile(w->fh, (const char *) cb->aio_buf + total, want, &step, &ov);
        } else {
            ok = ReadFile(w->fh, (char *) cb->aio_buf + total, want, &step, &ov);
        }
        if (!ok) {
            DWORD err = GetLastError();
            cb->_aio_xfer = total;
            return err;
        }
        if (0 == step) {
            break;
        }
        total += step;
    }
    cb->_aio_xfer = total;
    return 0;
}

static DWORD WINAPI aio_worker(LPVOID arg)
{
    aio_work_t *w = (aio_work_t *) arg;
    struct aiocb *cb = w->cb;
    DWORD err = aio_do_io(w);

    if (0 != err) {
    }
    cb->_aio_error = err;
    InterlockedExchange(&cb->_aio_state, AIO_DONE);
    SetEvent(cb->_aio_ov.hEvent);
    free(w);
    return 0;
}

static int aio_issue(struct aiocb *cb, BOOL is_write)
{
    aio_work_t *w;
    HANDLE fh;

    if (NULL == cb || NULL == cb->aio_buf) {
        errno = EINVAL;
        return -1;
    }
    fh = (HANDLE) _get_osfhandle(cb->aio_fildes);
    if (INVALID_HANDLE_VALUE == fh || NULL == fh) {
        errno = EBADF;
        return -1;
    }
    cb->_aio_ov.hEvent = CreateEventW(NULL, TRUE, FALSE, NULL);
    if (NULL == cb->_aio_ov.hEvent) {
        errno = ENOMEM;
        return -1;
    }
    w = (aio_work_t *) malloc(sizeof(*w));
    if (NULL == w) {
        CloseHandle(cb->_aio_ov.hEvent);
        cb->_aio_ov.hEvent = NULL;
        errno = ENOMEM;
        return -1;
    }
    w->cb = cb;
    w->fh = fh;
    w->write = is_write;

    cb->_aio_xfer = 0;
    cb->_aio_error = 0;
    InterlockedExchange(&cb->_aio_state, AIO_PENDING);

    cb->_aio_worker = (HANDLE) _beginthreadex(NULL, 0, aio_worker, w, 0, NULL);
    if (0 == cb->_aio_worker || INVALID_HANDLE_VALUE == cb->_aio_worker) {
        free(w);
        CloseHandle(cb->_aio_ov.hEvent);
        cb->_aio_ov.hEvent = NULL;
        InterlockedExchange(&cb->_aio_state, AIO_IDLE);
        errno = EAGAIN;   /* thread pool exhausted, like POSIX */
        return -1;
    }
    return 0;
}

static void aio_cleanup(struct aiocb *cb)
{
    if (NULL != cb->_aio_worker && INVALID_HANDLE_VALUE != cb->_aio_worker) {
        CloseHandle(cb->_aio_worker);
        cb->_aio_worker = NULL;
    }
    if (NULL != cb->_aio_ov.hEvent) {
        CloseHandle(cb->_aio_ov.hEvent);
        cb->_aio_ov.hEvent = NULL;
    }
}

OPAL_WIN32_DECLSPEC int aio_read(struct aiocb *cb)
{
    return aio_issue(cb, FALSE);
}

OPAL_WIN32_DECLSPEC int aio_write(struct aiocb *cb)
{
    return aio_issue(cb, TRUE);
}

OPAL_WIN32_DECLSPEC int aio_error(const struct aiocb *cb)
{
    LONG st;

    if (NULL == cb) {
        return EINVAL;
    }
    st = cb->_aio_state;
    if (AIO_DONE == st) {
        return (0 == cb->_aio_error)
                   ? 0
                   : ((ERROR_OPERATION_ABORTED == cb->_aio_error) ? ECANCELED : EIO);
    }
    if (AIO_PENDING == st) {
        if (NULL != cb->_aio_ov.hEvent && WAIT_OBJECT_0 == WaitForSingleObject(cb->_aio_ov.hEvent, 0)) {
            return (0 == cb->_aio_error)
                       ? 0
                       : ((ERROR_OPERATION_ABORTED == cb->_aio_error) ? ECANCELED : EIO);
        }
        return EINPROGRESS;
    }
    return EINVAL;
}

OPAL_WIN32_DECLSPEC ssize_t aio_return(struct aiocb *cb)
{
    ssize_t ret;

    if (NULL == cb) {
        errno = EINVAL;
        return -1;
    }
    if (AIO_PENDING == cb->_aio_state && NULL != cb->_aio_ov.hEvent) {
        WaitForSingleObject(cb->_aio_ov.hEvent, INFINITE);
    }
    if (0 != cb->_aio_error) {
        errno = (ERROR_OPERATION_ABORTED == cb->_aio_error) ? ECANCELED : EIO;
        ret = -1;
    } else {
        ret = (ssize_t) cb->_aio_xfer;
    }
    aio_cleanup(cb);
    InterlockedExchange(&cb->_aio_state, AIO_IDLE);
    return ret;
}

OPAL_WIN32_DECLSPEC int aio_suspend(const struct aiocb *const list[], int n,
                                    const struct timespec *timeout)
{
    HANDLE evs[63];
    int cnt = 0, i;
    DWORD wait_ms = INFINITE;
    DWORD rc;

    if (NULL == list || n <= 0) {
        errno = EINVAL;
        return -1;
    }
    if (NULL != timeout) {
        LONGLONG ns = (LONGLONG) timeout->tv_sec * 1000000000LL + timeout->tv_nsec;
        if (ns < 0) {
            errno = EINVAL;
            return -1;
        }
        wait_ms = (DWORD) ((ns + 999999LL) / 1000000LL);
    }
    for (i = 0; i < n && cnt < 63; ++i) {
        const struct aiocb *cb = list[i];
        if (NULL == cb) {
            continue;
        }
        if (AIO_PENDING != cb->_aio_state) {
            return 0;   /* something already finished */
        }
        if (NULL != cb->_aio_ov.hEvent) {
            evs[cnt++] = cb->_aio_ov.hEvent;
        }
    }
    if (0 == cnt) {
        return 0;
    }
    /* WaitAny on the posted events; >63 outstanding ops are re-checked
     * once the first batch signals (fbtl posts in much smaller chunks). */
    for (;;) {
        int batch = (cnt > 63) ? 63 : cnt;
        rc = WaitForMultipleObjects((DWORD) batch, evs, FALSE,
                                    (wait_ms > 100) ? 100 : wait_ms);
        if (WAIT_FAILED == rc) {
            errno = EIO;
            return -1;
        }
        if (WAIT_TIMEOUT != rc) {
            return 0;
        }
        if (NULL != timeout && wait_ms <= 100) {
            errno = EAGAIN;
            return -1;
        }
        if (NULL != timeout) {
            wait_ms -= 100;
        }
        for (i = 0; i < n; ++i) {
            if (NULL != list[i] && AIO_PENDING != list[i]->_aio_state) {
                return 0;
            }
        }
    }
}

OPAL_WIN32_DECLSPEC int aio_cancel(int fd, struct aiocb *cb)
{
    (void) fd;
    if (NULL == cb) {
        return AIO_ALLDONE;
    }
    if (AIO_PENDING != cb->_aio_state) {
        return AIO_ALLDONE;
    }
    /* A sync I/O already running in the worker cannot be yanked out of
     * the kernel; POSIX allows AIO_NOTCANCELED for exactly this case. */
    return AIO_NOTCANCELED;
}

OPAL_WIN32_DECLSPEC int aio_fsync(int op, struct aiocb *cb)
{
    (void) op;
    if (NULL == cb || AIO_PENDING == cb->_aio_state) {
        errno = EAGAIN;
        return -1;
    }
    if (!FlushFileBuffers((HANDLE) _get_osfhandle(cb->aio_fildes))) {
        errno = EIO;
        return -1;
    }
    return 0;
}

OPAL_WIN32_DECLSPEC int lio_listio(int mode, struct aiocb *const list[], int n,
                                   struct sigevent *sig)
{
    int i, rc = 0;
    (void) sig;

    if (NULL == list || n <= 0) {
        errno = EINVAL;
        return -1;
    }
    for (i = 0; i < n; ++i) {
        struct aiocb *cb = list[i];
        int r;
        if (NULL == cb || LIO_NOP == cb->aio_lio_opcode) {
            continue;
        }
        r = (LIO_WRITE == cb->aio_lio_opcode) ? aio_write(cb) : aio_read(cb);
        if (0 != r) {
            rc = -1;
        }
    }
    if (LIO_WAIT == mode) {
        for (i = 0; i < n; ++i) {
            if (NULL != list[i] && LIO_NOP != list[i]->aio_lio_opcode) {
                aio_return(list[i]);
            }
        }
    }
    return rc;
}

#endif /* _WIN32 */
