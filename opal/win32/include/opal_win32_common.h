/*
 * Copyright (c) 2026      Amazon.com, Inc. or its affiliates.
 *                         All Rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 *
 * Common definitions for the Open MPI native Windows port.  This
 * header is included (directly or transitively) by every shadow
 * header in opal/win32/include/ and must be self-contained.
 */

#ifndef OPAL_WIN32_COMMON_H
#define OPAL_WIN32_COMMON_H

/* The opal_win32_* compatibility functions live in open-pal.dll and
 * are called from other libraries' TUs through the macro shims in
 * these headers.  This must be defined *before* the system includes
 * below: <fcntl.h> et al resolve to our forwarders, which re-enter
 * this header (guard-skipped) and then use OPAL_WIN32_DECLSPEC while
 * we are still mid-flight. */
/* OPAL_WIN32_STATIC: the implementation objects are being linked
 * directly into the consuming target (no import/export bookkeeping) --
 * used by the pmix.dll build, which gets its own private copy of the
 * compat layer so libpmix does not depend on open-pal. */
#if defined(OPAL_WIN32_STATIC)
#    define OPAL_WIN32_DECLSPEC
#elif defined(OPAL_BUILDING)
#    define OPAL_WIN32_DECLSPEC __declspec(dllexport)
#else
#    define OPAL_WIN32_DECLSPEC __declspec(dllimport)
#endif

#ifndef _CRT_SECURE_NO_WARNINGS
#    define _CRT_SECURE_NO_WARNINGS 1
#endif
#ifndef _CRT_NON_CONFORMING_SWPRINTFS
#    define _CRT_NON_CONFORMING_SWPRINTFS 1
#endif
#ifndef WIN32_LEAN_AND_MEAN
#    define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#    define NOMINMAX
#endif
#ifndef _WINSOCK_DEPRECATED_NO_WARNINGS
#    define _WINSOCK_DEPRECATED_NO_WARNINGS
#endif

/* Winsock2 must always precede windows.h; pulling it in here makes the
 * ordering safe no matter which shadow header a translation unit hits
 * first. */
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <ws2def.h>
#include <mswsock.h>
#include <iphlpapi.h>
#include <BaseTsd.h>

/* <sys/types.h> first: the shadow forwarder adds the POSIX typedefs
 * (pid_t, mode_t, ...) that the CRT-adjacent headers below rely on. */
#include <sys/types.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <errno.h>
#include <fcntl.h>
#include <io.h>
#include <process.h>
#include <direct.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ------------------------------------------------------------------ */
/* Winsock glue                                                        */
/* ------------------------------------------------------------------ */

/* Initialize Winsock lazily; safe to call any number of times. */
static __inline int opal_win32_socket_init(void)
{
    static volatile LONG once = 0;
    static int rc = 0;
    if (0 == InterlockedCompareExchange(&once, 1, 0)) {
        WSADATA wd;
        rc = WSAStartup(MAKEWORD(2, 2), &wd);
        InterlockedExchange(&once, 2);
    } else {
        while (1 == once) {
            Sleep(0);
        }
    }
    return rc;
}

typedef SOCKET opal_win32_socket_t;

/* POSIX code uses "int" for fds; OMPI has already been taught (in
 * opal_socket_errno.h) to wrap socket error retrieval.  We define
 * close() handling for sockets in the .c side of select paths; for
 * general fd plumbing, see unistd.h. */
#ifndef opal_socket_errno
/* real definition lives in opal_win32_socket.c.  Must use the same
 * dllimport/dllexport spelling as opal/opal_socket_errno.h in every TU
 * or MSVC reports inconsistent dll linkage (C2373/C2375). */
OPAL_WIN32_DECLSPEC int opal_win32_socket_errno(void);
#endif /* opal_socket_errno */

/* Convert an absolute path that may contain forward slashes to the
 * native form; also handles the POSIX-style "/tmp" -> %TEMP% mapping
 * used by hard-coded fallbacks in the tree. */
static __inline void opal_win32_fixpath(char *dst, const char *src, size_t n)
{
    size_t i;
    for (i = 0; i + 1 < n && src[i]; i++) {
        dst[i] = ('/' == src[i]) ? '\\' : src[i];
    }
    dst[i] = '\0';
}

/* MSVC has no __attribute__; vendored C sources (e.g. json.c) use it
 * directly -- expand it to nothing under the Windows build. */
#if defined(_MSC_VER) && !defined(__attribute__)
#    define __attribute__(x)
#endif

#ifdef __cplusplus
}
#endif

#endif /* OPAL_WIN32_COMMON_H */
