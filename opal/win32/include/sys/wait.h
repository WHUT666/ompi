/*
 * Copyright (c) 2026      Amazon.com, Inc. or its affiliates.
 *                         All Rights reserved.
 * $COPYRIGHT$
 * $HEADER$
 * POSIX <sys/wait.h> replacement for the native Windows build.
 * waitpid() is implemented over a registry of spawned child handles
 * maintained by the compat layer's process-spawn helpers.
 */
#ifndef OPAL_WIN32_SYS_WAIT_H
#define OPAL_WIN32_SYS_WAIT_H

#include "opal_win32_common.h"

#ifdef __cplusplus
extern "C" {
#endif

#define WNOHANG    0x1
#define WUNTRACED  0x2
#define WCONTINUED 0x8

/* status encoding:  normal exit -> (code << 8); terminated by signal
 * -> sig.  Matches the glibc layout the code base tests. */
#define WIFEXITED(s)      (((s) & 0xff) == 0)
#define WIFSIGNALED(s)    (((s) & 0xff) != 0 && ((s) & 0xff) != 0x7f)
#define WEXITSTATUS(s)    (((s) >> 8) & 0xff)
#define WTERMSIG(s)       ((s) & 0x7f)
#define WIFSTOPPED(s)     (((s) & 0xff) == 0x7f)
#define WSTOPSIG(s)       (((s) >> 8) & 0xff)
#define WIFCONTINUED(s)   ((s) == 0xffff)
#define WCOREDUMP(s)      ((s) & 0x80)
#define W_EXITCODE(r, s)  (((r) & 0xff) << 8 | (s) & 0xff)

OPAL_WIN32_DECLSPEC pid_t opal_win32_waitpid(pid_t pid, int *status, int options);
OPAL_WIN32_DECLSPEC pid_t opal_win32_wait(int *status);
OPAL_WIN32_DECLSPEC int   opal_win32_waitid(int idtype, int id, void *infop, int options);
OPAL_WIN32_DECLSPEC pid_t opal_win32_wait3(int *status, int options, void *rusage);
OPAL_WIN32_DECLSPEC pid_t opal_win32_wait4(pid_t pid, int *status, int options, void *rusage);

#define waitpid opal_win32_waitpid
#define wait    opal_win32_wait
#define waitid  opal_win32_waitid
#define wait3   opal_win32_wait3
#define wait4   opal_win32_wait4

/* Register/unregister a child process handle for waitpid().  Called by
 * the spawn helpers in the compat layer and by prrte's odls/win32. */
OPAL_WIN32_DECLSPEC void opal_win32_register_child(pid_t pid, HANDLE handle);
OPAL_WIN32_DECLSPEC void opal_win32_unregister_child(pid_t pid);

#ifdef __cplusplus
}
#endif

#endif /* OPAL_WIN32_SYS_WAIT_H */
