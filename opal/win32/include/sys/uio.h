/*
 * Copyright (c) 2026      Amazon.com, Inc. or its affiliates.
 *                         All Rights reserved.
 * $COPYRIGHT$
 * $HEADER$
 * POSIX <sys/uio.h> replacement for the native Windows build.
 */
#ifndef OPAL_WIN32_SYS_UIO_H
#define OPAL_WIN32_SYS_UIO_H

#include "opal_win32_common.h"

struct iovec {
    void  *iov_base;
    size_t iov_len;
};

#ifdef __cplusplus
extern "C" {
#endif

OPAL_WIN32_DECLSPEC ssize_t opal_win32_readv(int fd, const struct iovec *iov, int iovcnt);
OPAL_WIN32_DECLSPEC ssize_t opal_win32_writev(int fd, const struct iovec *iov, int iovcnt);

/* process_vm_readv/writev are implemented over ReadProcessMemory /
 * WriteProcessMemory; the smsc "windows" component uses them for
 * single-copy transfers between local processes. */
OPAL_WIN32_DECLSPEC ssize_t opal_win32_process_vm_readv(pid_t pid, const struct iovec *lvec,
                                    unsigned long liovcnt, const struct iovec *rvec,
                                    unsigned long riovcnt, unsigned long flags);
OPAL_WIN32_DECLSPEC ssize_t opal_win32_process_vm_writev(pid_t pid, const struct iovec *lvec,
                                     unsigned long liovcnt, const struct iovec *rvec,
                                     unsigned long riovcnt, unsigned long flags);

#define readv              opal_win32_readv
#define writev             opal_win32_writev
#define process_vm_readv   opal_win32_process_vm_readv
#define process_vm_writev  opal_win32_process_vm_writev

#ifdef __cplusplus
}
#endif

#endif /* OPAL_WIN32_SYS_UIO_H */
