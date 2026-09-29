/*
 * Copyright (c) 2026      Amazon.com, Inc. or its affiliates.
 *                         All Rights reserved.
 * $COPYRIGHT$
 * $HEADER$
 * <pty.h> stub for the native Windows build.  Windows has no POSIX
 * ptys; prrte's iof framework uses pipes there instead.
 */
#ifndef OPAL_WIN32_PTY_H
#define OPAL_WIN32_PTY_H

#include "opal_win32_common.h"
#include <termios.h>

struct winsize;

OPAL_WIN32_DECLSPEC int opal_win32_openpty(int *amaster, int *aslave, char *name,
                       const struct termios *termp, const void *winp);
OPAL_WIN32_DECLSPEC int opal_win32_forkpty(int *amaster, char *name, const struct termios *termp,
                       const void *winp);
OPAL_WIN32_DECLSPEC int opal_win32_login_tty(int fd);

#define openpty   opal_win32_openpty
#define forkpty   opal_win32_forkpty
#define login_tty opal_win32_login_tty

#endif /* OPAL_WIN32_PTY_H */
