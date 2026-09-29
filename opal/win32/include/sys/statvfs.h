/*
 * Copyright (c) 2026      Amazon.com, Inc. or its affiliates.
 *                         All Rights reserved.
 * $COPYRIGHT$
 * $HEADER$
 * POSIX <sys/statvfs.h> replacement for the native Windows build.
 */
#ifndef OPAL_WIN32_SYS_STATVFS_H
#define OPAL_WIN32_SYS_STATVFS_H

#include "opal_win32_common.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _FSBLKCNT_T_DEFINED
typedef unsigned long long fsblkcnt_t;
typedef unsigned long long fsfilcnt_t;
#    define _FSBLKCNT_T_DEFINED
#endif

struct statvfs {
    unsigned long  f_bsize;
    unsigned long  f_frsize;
    fsblkcnt_t     f_blocks;
    fsblkcnt_t     f_bfree;
    fsblkcnt_t     f_bavail;
    fsfilcnt_t     f_files;
    fsfilcnt_t     f_ffree;
    fsfilcnt_t     f_favail;
    unsigned long  f_fsid;
    unsigned long  f_flag;
    unsigned long  f_namemax;
};

#define ST_RDONLY      0x01
#define ST_NOSUID      0x02
#define ST_NODEV       0x04
#define ST_NOEXEC      0x08
#define ST_SYNCHRONOUS 0x10
#define ST_MANDLOCK    0x40
#define ST_WRITE       0x80
#define ST_APPEND      0x100
#define ST_IMMUTABLE   0x200
#define ST_NOATIME     0x400
#define ST_NODIRATIME  0x800

/* declared literally: the CRT has no statvfs so there is no symbol
 * collision, and a statvfs macro would rewrite "struct statvfs" in
 * callers too */
OPAL_WIN32_DECLSPEC int statvfs(const char *path, struct statvfs *buf);
OPAL_WIN32_DECLSPEC int fstatvfs(int fd, struct statvfs *buf);

#ifdef __cplusplus
}
#endif

#endif /* OPAL_WIN32_SYS_STATVFS_H */
