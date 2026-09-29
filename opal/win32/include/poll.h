/*
 * Copyright (c) 2026      Amazon.com, Inc. or its affiliates.
 *                         All Rights reserved.
 * $COPYRIGHT$
 * $HEADER$
 * POSIX <poll.h> replacement for the native Windows build.
 * winsock2.h already provides struct pollfd and the POLL* bits (for
 * WSAPoll); we add nfds_t and our poll() wrapper which is implemented
 * over select() (WSAPoll is unreliable on older Windows releases).
 */
#ifndef OPAL_WIN32_POLL_H
#define OPAL_WIN32_POLL_H

#include "opal_win32_common.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned long nfds_t;

#ifndef POLLRDHUP
#    define POLLRDHUP 0x2000
#endif
#ifndef POLLMSG
#    define POLLMSG 0x0400
#endif

OPAL_WIN32_DECLSPEC int opal_win32_poll(struct pollfd *fds, nfds_t nfds, int timeout);

#define poll(fds, nfds, timeout) opal_win32_poll((fds), (nfds), (timeout))

#ifdef __cplusplus
}
#endif

#endif /* OPAL_WIN32_POLL_H */
