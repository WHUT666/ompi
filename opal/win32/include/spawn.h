/*
 * Copyright (c) 2026      Amazon.com, Inc. or its affiliates.
 *                         All Rights reserved.
 * $COPYRIGHT$
 * $HEADER$
 * POSIX <spawn.h> replacement for the native Windows build.
 */
#ifndef OPAL_WIN32_SPAWN_H
#define OPAL_WIN32_SPAWN_H

#include "opal_win32_common.h"
#include <sys/types.h>
#include <sched.h>
#include <signal.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int flags;
    int pgrp;
    sigset_t sd;
    sigset_t mask;
    int schedpolicy;
    struct sched_param schedparam;
    int resetids;
    HANDLE job;
    char *chdir;
    char *cwd;
} posix_spawnattr_t;

typedef struct {
    int  _allocated;
    int  _used;
    void *_actions;
} posix_spawn_file_actions_t;

OPAL_WIN32_DECLSPEC int posix_spawn(pid_t *pid, const char *path,
                const posix_spawn_file_actions_t *file_actions,
                const posix_spawnattr_t *attrp, char *const argv[],
                char *const envp[]);
OPAL_WIN32_DECLSPEC int posix_spawnp(pid_t *pid, const char *file,
                 const posix_spawn_file_actions_t *file_actions,
                 const posix_spawnattr_t *attrp, char *const argv[],
                 char *const envp[]);

OPAL_WIN32_DECLSPEC int posix_spawn_file_actions_init(posix_spawn_file_actions_t *fa);
OPAL_WIN32_DECLSPEC int posix_spawn_file_actions_destroy(posix_spawn_file_actions_t *fa);
OPAL_WIN32_DECLSPEC int posix_spawn_file_actions_addopen(posix_spawn_file_actions_t *fa, int fd,
                                     const char *path, int oflag, mode_t mode);
OPAL_WIN32_DECLSPEC int posix_spawn_file_actions_addclose(posix_spawn_file_actions_t *fa, int fd);
OPAL_WIN32_DECLSPEC int posix_spawn_file_actions_adddup2(posix_spawn_file_actions_t *fa, int fd,
                                     int newfd);

OPAL_WIN32_DECLSPEC int posix_spawnattr_init(posix_spawnattr_t *attr);
OPAL_WIN32_DECLSPEC int posix_spawnattr_destroy(posix_spawnattr_t *attr);
OPAL_WIN32_DECLSPEC int posix_spawnattr_getflags(const posix_spawnattr_t *attr, short *flags);
OPAL_WIN32_DECLSPEC int posix_spawnattr_setflags(posix_spawnattr_t *attr, short flags);
OPAL_WIN32_DECLSPEC int posix_spawnattr_getpgroup(const posix_spawnattr_t *attr, pid_t *pgroup);
OPAL_WIN32_DECLSPEC int posix_spawnattr_setpgroup(posix_spawnattr_t *attr, pid_t pgroup);
OPAL_WIN32_DECLSPEC int posix_spawnattr_getsigmask(const posix_spawnattr_t *attr, sigset_t *mask);
OPAL_WIN32_DECLSPEC int posix_spawnattr_setsigmask(posix_spawnattr_t *attr, const sigset_t *mask);
OPAL_WIN32_DECLSPEC int posix_spawnattr_getsigdefault(const posix_spawnattr_t *attr, sigset_t *def);
OPAL_WIN32_DECLSPEC int posix_spawnattr_setsigdefault(posix_spawnattr_t *attr, const sigset_t *def);
OPAL_WIN32_DECLSPEC int posix_spawnattr_getschedpolicy(const posix_spawnattr_t *attr, int *policy);
OPAL_WIN32_DECLSPEC int posix_spawnattr_setschedpolicy(posix_spawnattr_t *attr, int policy);
OPAL_WIN32_DECLSPEC int posix_spawnattr_getschedparam(const posix_spawnattr_t *attr,
                                  struct sched_param *param);
OPAL_WIN32_DECLSPEC int posix_spawnattr_setschedparam(posix_spawnattr_t *attr,
                                  const struct sched_param *param);

#define POSIX_SPAWN_RESETIDS       0x01
#define POSIX_SPAWN_SETPGROUP      0x02
#define POSIX_SPAWN_SETSCHEDPARAM  0x04
#define POSIX_SPAWN_SETSCHEDULER   0x08
#define POSIX_SPAWN_SETSIGDEF      0x10
#define POSIX_SPAWN_SETSIGMASK     0x20

/* the spawn helper used by prrte's odls/win32 and PMIx' pfexec */
OPAL_WIN32_DECLSPEC pid_t opal_win32_spawn_process(const char *path, char *const argv[],
                               char *const envp[], const char *cwd,
                               HANDLE hStdin, HANDLE hStdout, HANDLE hStderr,
                               int suspended, HANDLE *outhandle, HANDLE *outthread);

#ifdef __cplusplus
}
#endif

#endif /* OPAL_WIN32_SPAWN_H */
