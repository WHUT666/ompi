/*
 * Copyright (c) 2026      Amazon.com, Inc. or its affiliates.
 *                         All Rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 *
 * Internal header for the Windows compatibility implementation files.
 * Pulls in every shadow header so the .c files see consistent
 * declarations; translation units implementing wrappers must #undef
 * the macro name they define (see opal_win32_socket.c).
 */

#ifndef OPAL_WIN32_H
#define OPAL_WIN32_H

#include "opal_config.h"

#ifdef _WIN32

#    include "opal_win32_common.h"

#    include <unistd.h>
#    include <sys/types.h>
#    include <sys/stat.h>
#    include <sys/utime.h>
#    include <sys/socket.h>
#    include <sys/time.h>
#    include <sys/select.h>
#    include <sys/uio.h>
#    include <sys/mman.h>
#    include <sys/wait.h>
#    include <sys/ioctl.h>
#    include <sys/param.h>
#    include <sys/resource.h>
#    include <sys/utsname.h>
#    include <sys/statvfs.h>
#    include <sys/times.h>
#    include <sys/file.h>
#    include <sys/ipc.h>
#    include <sys/shm.h>
#    include <sys/syscall.h>
#    include <spawn.h>
#    include <limits.h>
#    include <net/if.h>
#    include <netdb.h>
#    include <netinet/in.h>
#    include <netinet/tcp.h>
#    include <arpa/inet.h>
#    include <ifaddrs.h>
#    include <poll.h>
#    include <dlfcn.h>
#    include <pthread.h>
#    include <sched.h>
#    include <semaphore.h>
#    include <libgen.h>
#    include <strings.h>
#    include <paths.h>
#    include <regex.h>
#    include <syslog.h>
#    include <dirent.h>
#    include <pwd.h>
#    include <grp.h>
#    include <termios.h>
#    include <fnmatch.h>
#    include <getopt.h>
#    include <endian.h>
#    include <execinfo.h>
#    include <pty.h>
#    include <fcntl.h>
#    include <signal.h>
#    include <stdio.h>
#    include <stdlib.h>
#    include <string.h>
#    include <errno.h>
#    include <limits.h>
#    include <malloc.h>
#    include <stdarg.h>
#    include <assert.h>

/* dirfd-relative resolution shared by the *at() implementations:
 * joins the directory behind dirfd (GetFinalPathNameByHandle on the CRT
 * handle) with name; AT_FDCWD and absolute names pass through. */
OPAL_WIN32_DECLSPEC int opal_win32_resolve_at(int dirfd, const char *name, char *out,
                          size_t outlen);

/* O_NONBLOCK bookkeeping for non-socket CRT fds (pipes).  Windows
 * anonymous pipes have no nonblocking mode; fds marked here get a
 * PeekNamedPipe guard inside opal_win32_read so iof's timer-driven pump
 * can poll them without ever blocking the progress thread. */
OPAL_WIN32_DECLSPEC void opal_win32_fd_set_nonblocking(int fd, int nb);
OPAL_WIN32_DECLSPEC int opal_win32_fd_is_nonblocking(int fd);

#endif /* _WIN32 */
#endif /* OPAL_WIN32_H */
