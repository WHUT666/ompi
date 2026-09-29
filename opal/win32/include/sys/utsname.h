/*
 * Copyright (c) 2026      Amazon.com, Inc. or its affiliates.
 *                         All Rights reserved.
 * $COPYRIGHT$
 * $HEADER$
 * POSIX <sys/utsname.h> replacement for the native Windows build.
 */
#ifndef OPAL_WIN32_SYS_UTSNAME_H
#define OPAL_WIN32_SYS_UTSNAME_H

#include "opal_win32_common.h"

#ifdef __cplusplus
extern "C" {
#endif

struct utsname {
    char sysname[65];
    char nodename[65];
    char release[65];
    char version[65];
    char machine[65];
    char domainname[65];
};

OPAL_WIN32_DECLSPEC int opal_win32_uname(struct utsname *buf);
#define uname opal_win32_uname

#ifdef __cplusplus
}
#endif

#endif /* OPAL_WIN32_SYS_UTSNAME_H */
