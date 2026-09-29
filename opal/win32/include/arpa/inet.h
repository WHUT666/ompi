/*
 * Copyright (c) 2026      Amazon.com, Inc. or its affiliates.
 *                         All Rights reserved.
 * $COPYRIGHT$
 * $HEADER$
 * POSIX <arpa/inet.h> replacement for the native Windows build.
 */
#ifndef OPAL_WIN32_ARPA_INET_H
#define OPAL_WIN32_ARPA_INET_H

#include "opal_win32_common.h"

#ifdef __cplusplus
extern "C" {
#endif

/* hton/ntoh/inet_addr/inet_ntoa/inet_pton/inet_ntop and the in*_addr
 * types come from winsock2.h/ws2tcpip.h included via
 * opal_win32_common.h. */

#define inet_aton(a, b) opal_win32_inet_aton((a), (b))
OPAL_WIN32_DECLSPEC int opal_win32_inet_aton(const char *cp, struct in_addr *inp);

OPAL_WIN32_DECLSPEC const char *opal_win32_inet_ntop(int af, const void *src, char *dst, socklen_t size);
OPAL_WIN32_DECLSPEC int         opal_win32_inet_pton(int af, const char *src, void *dst);

#ifdef inet_ntop
#    undef inet_ntop
#endif
#ifdef inet_pton
#    undef inet_pton
#endif
#define inet_ntop opal_win32_inet_ntop
#define inet_pton opal_win32_inet_pton

#ifdef __cplusplus
}
#endif

#endif /* OPAL_WIN32_ARPA_INET_H */
