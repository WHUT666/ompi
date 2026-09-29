/*
 * Copyright (c) 2026      Amazon.com, Inc. or its affiliates.
 *                         All Rights reserved.
 * $COPYRIGHT$
 * $HEADER$
 * BSD <sys/file.h> replacement for the native Windows build --
 * flock() over LockFileEx/UnlockFileEx.
 */
#ifndef OPAL_WIN32_SYS_FILE_H
#define OPAL_WIN32_SYS_FILE_H

#include "opal_win32_common.h"

#ifdef __cplusplus
extern "C" {
#endif

#define LOCK_SH 0x01
#define LOCK_EX 0x02
#define LOCK_NB 0x04
#define LOCK_UN 0x08

OPAL_WIN32_DECLSPEC int opal_win32_flock(int fd, int operation);
#define flock opal_win32_flock

#ifdef __cplusplus
}
#endif

#endif /* OPAL_WIN32_SYS_FILE_H */
