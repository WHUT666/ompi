/*
 * Copyright (c) 2026      Amazon.com, Inc. or its affiliates.
 *                         All Rights reserved.
 * $COPYRIGHT$
 * $HEADER$
 * POSIX <sys/ioctl.h> replacement for the native Windows build.
 */
#ifndef OPAL_WIN32_SYS_IOCTL_H
#define OPAL_WIN32_SYS_IOCTL_H

#include "opal_win32_common.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Winsock ioctlsocket() commands we support */
#ifndef FIONBIO
#    define FIONBIO 0x8004667eUL
#endif
#ifndef FIONREAD
#    define FIONREAD 0x4004667fUL
#endif
#ifndef SIOCATMARK
#    define SIOCATMARK 0x40046675UL
#endif

/* terminal-size ioctl support */
struct winsize {
    unsigned short ws_row;
    unsigned short ws_col;
    unsigned short ws_xpixel;
    unsigned short ws_ypixel;
};
#define TIOCGWINSZ 0x5413
#define TIOCSWINSZ 0x5414
#define TIOCSCTTY  0x540E
#define TCGETS     0x5401
#define TCSETS     0x5402
#define TCSETSW    0x5403
#define TCSETSF    0x5404
#define TIOCSPGRP  0x5410
#define TIOCGPGRP  0x540F
#define TIOCSBRK   0x5427
#define TIOCSIG    0x40045436UL

OPAL_WIN32_DECLSPEC int opal_win32_ioctl(int fd, unsigned long request, ...);

#ifndef ioctl
#    define ioctl opal_win32_ioctl
#endif

#ifdef __cplusplus
}
#endif

#endif /* OPAL_WIN32_SYS_IOCTL_H */
