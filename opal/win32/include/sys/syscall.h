/*
 * Copyright (c) 2026      Amazon.com, Inc. or its affiliates.
 *                         All Rights reserved.
 * $COPYRIGHT$
 * $HEADER$
 * <sys/syscall.h> stub for the native Windows build.
 */
#ifndef OPAL_WIN32_SYS_SYSCALL_H
#define OPAL_WIN32_SYS_SYSCALL_H

#include "opal_win32_common.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SYS_gettid   186
#define SYS_getpid   39
#define SYS_getppid  110
#define SYS_memfd_create 319

OPAL_WIN32_DECLSPEC long opal_win32_syscall(long number, ...);
#define syscall opal_win32_syscall

#ifdef __cplusplus
}
#endif

#endif /* OPAL_WIN32_SYS_SYSCALL_H */
