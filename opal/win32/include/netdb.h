/*
 * Copyright (c) 2026      Amazon.com, Inc. or its affiliates.
 *                         All Rights reserved.
 * $COPYRIGHT$
 * $HEADER$
 * POSIX <netdb.h> replacement for the native Windows build.
 * Winsock2 provides getaddrinfo()/getnameinfo()/hostent; we add the
 * pieces it lacks.
 */
#ifndef OPAL_WIN32_NETDB_H
#define OPAL_WIN32_NETDB_H

#include "opal_win32_common.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef NI_MAXHOST
#    define NI_MAXHOST 1025
#endif
#ifndef NI_MAXSERV
#    define NI_MAXSERV 32
#endif
#ifndef HOST_NOT_FOUND
#    define HOST_NOT_FOUND WSAHOST_NOT_FOUND
#endif
#ifndef TRY_AGAIN
#    define TRY_AGAIN WSATRY_AGAIN
#endif
#ifndef NO_RECOVERY
#    define NO_RECOVERY WSANO_RECOVERY
#endif
#ifndef NO_DATA
#    define NO_DATA WSANO_DATA
#endif
#ifndef NO_ADDRESS
#    define NO_ADDRESS WSANO_DATA
#endif

/* h_errno is meaningless under Winsock; keep the name defined */
#ifndef h_errno
#    define h_errno (WSAGetLastError())
#endif

/* AI_* and NI_* flags are in ws2tcpip.h (via opal_win32_common.h).  The
 * only APIs we wrap are the re-entrant host lookup variants that POSIX
 * code expects. */
OPAL_WIN32_DECLSPEC struct hostent *opal_win32_gethostbyname(const char *name);
OPAL_WIN32_DECLSPEC struct hostent *opal_win32_gethostbyaddr(const void *addr, int len, int type);
OPAL_WIN32_DECLSPEC int opal_win32_gethostname(char *name, size_t len);

#ifndef gethostbyname
#    define gethostbyname  opal_win32_gethostbyname
#endif
#ifndef gethostbyaddr
#    define gethostbyaddr  opal_win32_gethostbyaddr
#endif

/* ensure the libc init has run */
#define getaddrinfo(n, s, h, r) \
    (opal_win32_socket_init(), getaddrinfo((n), (s), (h), (r)))
#define getnameinfo(a, l, nh, nhl, sh, shl, f) \
    (opal_win32_socket_init(), getnameinfo((a), (l), (nh), (nhl), (sh), (shl), (f)))

#ifdef __cplusplus
}
#endif

#endif /* OPAL_WIN32_NETDB_H */
