/*
 * Copyright (c) 2026      Amazon.com, Inc. or its affiliates.
 *                         All Rights reserved.
 * $COPYRIGHT$
 * $HEADER$
 * POSIX <sys/shm.h> stub for the native Windows build -- all calls
 * fail; the win32 shared memory component is used instead.
 */
#ifndef OPAL_WIN32_SYS_SHM_H
#define OPAL_WIN32_SYS_SHM_H

#include "opal_win32_common.h"
#include <sys/ipc.h>

#define SHM_R      0x100
#define SHM_W      0x80
#define SHM_RDONLY 0x1000
#define SHM_RND    0x2000
#define SHMLBA     4096

struct shmid_ds {
    struct ipc_perm shm_perm;
    size_t          shm_segsz;
    pid_t           shm_lpid;
    pid_t           shm_cpid;
    unsigned short  shm_nattch;
    unsigned long   shm_atime;
    unsigned long   shm_dtime;
    unsigned long   shm_ctime;
};

OPAL_WIN32_DECLSPEC int shmget(key_t key, size_t size, int shmflg);
void *shmat(int shmid, const void *shmaddr, int shmflg);
OPAL_WIN32_DECLSPEC int shmdt(const void *shmaddr);
OPAL_WIN32_DECLSPEC int shmctl(int shmid, int cmd, struct shmid_ds *buf);

#endif /* OPAL_WIN32_SYS_SHM_H */
