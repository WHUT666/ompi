/*
 * Copyright (c) 2026      Amazon.com, Inc. or its affiliates.
 *                         All Rights reserved.
 * $COPYRIGHT$
 * $HEADER$
 * POSIX <sys/mman.h> replacement for the native Windows build,
 * implemented over CreateFileMapping/MapViewOfFile.
 */
#ifndef OPAL_WIN32_SYS_MMAN_H
#define OPAL_WIN32_SYS_MMAN_H

#include "opal_win32_common.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PROT_NONE   0x0
#define PROT_READ   0x1
#define PROT_WRITE  0x2
#define PROT_EXEC   0x4

#define MAP_SHARED   0x0001
#define MAP_PRIVATE  0x0002
#define MAP_FIXED    0x0010
#define MAP_ANON     0x0020
#define MAP_ANONYMOUS MAP_ANON
#define MAP_NORESERVE 0x0040
#define MAP_LOCKED   0x0080
#define MAP_POPULATE 0x0100
#define MAP_32BIT    0x0200
#define MAP_HUGETLB  0x0400
#define MAP_SYNC     0x0800
#define MAP_FIXED_NOREPLACE 0x1000

#define MAP_FAILED ((void *) -1)

/* madvise advice values (no-ops, kept for API compatibility) */
#define MADV_NORMAL      0
#define MADV_RANDOM      1
#define MADV_SEQUENTIAL  2
#define MADV_WILLNEED    3
#define MADV_DONTNEED    4
#define MADV_FREE        5
#define MADV_MERGEABLE   6
#define MADV_DONTFORK    7
#define MADV_HUGEPAGE    8
#define MADV_NOHUGEPAGE  9

#define MS_ASYNC       0x1
#define MS_SYNC        0x2
#define MS_INVALIDATE  0x4

#define MCL_CURRENT    0x1
#define MCL_FUTURE     0x2
#define MCL_ONFAULT    0x4

OPAL_WIN32_DECLSPEC void *opal_win32_mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset);
OPAL_WIN32_DECLSPEC int   opal_win32_munmap(void *addr, size_t length);
OPAL_WIN32_DECLSPEC int   opal_win32_mprotect(void *addr, size_t len, int prot);
OPAL_WIN32_DECLSPEC int   opal_win32_msync(void *addr, size_t len, int flags);
OPAL_WIN32_DECLSPEC int   opal_win32_madvise(void *addr, size_t len, int advice);
OPAL_WIN32_DECLSPEC int   opal_win32_mlock(const void *addr, size_t len);
OPAL_WIN32_DECLSPEC int   opal_win32_munlock(const void *addr, size_t len);
OPAL_WIN32_DECLSPEC int   opal_win32_mlockall(int flags);
OPAL_WIN32_DECLSPEC int   opal_win32_munlockall(void);
OPAL_WIN32_DECLSPEC int   opal_win32_mincore(void *addr, size_t length, unsigned char *vec);
OPAL_WIN32_DECLSPEC int   opal_win32_shm_open(const char *name, int oflag, mode_t mode);
OPAL_WIN32_DECLSPEC int   opal_win32_shm_unlink(const char *name);
OPAL_WIN32_DECLSPEC void *opal_win32_mmap64(void *addr, size_t length, int prot, int flags, int fd,
                        long long offset);
OPAL_WIN32_DECLSPEC int   opal_win32_posix_madvise(void *addr, size_t len, int advice);

#define mmap            opal_win32_mmap
#define mmap64          opal_win32_mmap64
#define munmap          opal_win32_munmap
#define mprotect        opal_win32_mprotect
#define msync           opal_win32_msync
#define madvise         opal_win32_madvise
#define posix_madvise   opal_win32_posix_madvise
#define mlock           opal_win32_mlock
#define munlock         opal_win32_munlock
#define mlockall        opal_win32_mlockall
#define munlockall      opal_win32_munlockall
#define mincore         opal_win32_mincore
#define shm_open        opal_win32_shm_open
#define shm_unlink      opal_win32_shm_unlink

#ifdef __cplusplus
}
#endif

#endif /* OPAL_WIN32_SYS_MMAN_H */
