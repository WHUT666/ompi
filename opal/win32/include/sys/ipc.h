/*
 * Copyright (c) 2026      Amazon.com, Inc. or its affiliates.
 *                         All Rights reserved.
 * $COPYRIGHT$
 * $HEADER$
 * POSIX <sys/ipc.h> stub for the native Windows build -- only used by
 * the SysV shared memory component (not built on Windows), kept so
 * shared headers compile.
 */
#ifndef OPAL_WIN32_SYS_IPC_H
#define OPAL_WIN32_SYS_IPC_H

#include "opal_win32_common.h"

#define IPC_CREAT  0x0200
#define IPC_EXCL   0x0400
#define IPC_NOWAIT 0x0800
#define IPC_PRIVATE 0
#define IPC_RMID   0
#define IPC_SET    1
#define IPC_STAT   2
#define IPC_INFO   3

struct ipc_perm {
    key_t  key;
    uid_t  uid;
    gid_t  gid;
    uid_t  cuid;
    gid_t  cgid;
    unsigned short mode;
};

OPAL_WIN32_DECLSPEC key_t ftok(const char *pathname, int proj_id);

#endif /* OPAL_WIN32_SYS_IPC_H */
