/*
 * Copyright (c) 2026      Amazon.com, Inc. or its affiliates.
 *                         All Rights reserved.
 * $COPYRIGHT$
 * $HEADER$
 * BSD <paths.h> replacement for the native Windows build.
 */
#ifndef OPAL_WIN32_PATHS_H
#define OPAL_WIN32_PATHS_H

/* These exist only so path-handling code compiles; Windows callers
 * should prefer opal_install_dirs / environment lookups. */
#define _PATH_DEV       "NUL:"
#define _PATH_DEVNULL   "NUL"
#define _PATH_DEVZERO   "NUL"
#define _PATH_BSHELL    "cmd.exe"
#define _PATH_CSHELL    "cmd.exe"
#define _PATH_DEFPATH   "C:\\Windows\\System32;C:\\Windows"
#define _PATH_STDPATH   "C:\\Windows\\System32;C:\\Windows"
#define _PATH_MAILDIR   "C:\\Temp"
#define _PATH_MAN       "C:\\Windows\\Help"
#define _PATH_PRESERVE  "C:\\Temp"
#define _PATH_SENDMAIL  "sendmail"
#define _PATH_TMP       "C:\\Windows\\Temp\\"
#define _PATH_TTY       "CON:"
#define _PATH_UTMP      "C:\\Temp\\utmp"
#define _PATH_WTMP      "C:\\Temp\\wtmp"
#define _PATH_VARTMP    "C:\\Windows\\Temp\\"
#define _PATH_VI        "notepad.exe"

#endif /* OPAL_WIN32_PATHS_H */
