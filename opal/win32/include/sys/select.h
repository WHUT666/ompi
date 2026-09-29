/*
 * Copyright (c) 2026      Amazon.com, Inc. or its affiliates.
 *                         All Rights reserved.
 * $COPYRIGHT$
 * $HEADER$
 * POSIX <sys/select.h> for the native Windows build.  Winsock select()
 * operates on SOCKET handles only; the signature matches POSIX.
 */
#ifndef OPAL_WIN32_SYS_SELECT_H
#define OPAL_WIN32_SYS_SELECT_H

#include "opal_win32_common.h"
#include <sys/time.h>

#ifdef __cplusplus
extern "C" {
#endif

/* fd_set and FD_* come from winsock2.h.  select() itself is a direct
 * Winsock call; passing CRT fds will just never be ready. */
static inline int opal_win32_select(int nfds, fd_set *readfds, fd_set *writefds,
                                    fd_set *exceptfds, struct timeval *timeout)
{
    opal_win32_socket_init();
    struct timeval *pt = (struct timeval *) timeout;
    return select(nfds, readfds, writefds, exceptfds, pt);
}

#define select(nfds, rd, wr, ex, to) opal_win32_select((nfds), (rd), (wr), (ex), (to))

#ifdef __cplusplus
}
#endif

#endif /* OPAL_WIN32_SYS_SELECT_H */
