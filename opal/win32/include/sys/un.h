/*
 * Copyright (c) 2026      Amazon.com, Inc. or its affiliates.
 *                         All Rights reserved.
 * $COPYRIGHT$
 * $HEADER$
 * POSIX <sys/un.h> for the native Windows build.  Windows 10 SDK
 * supplies <afunix.h> with sockaddr_un and AF_UNIX support; we wrap it.
 */
#ifndef OPAL_WIN32_SYS_UN_H
#define OPAL_WIN32_SYS_UN_H

#include "opal_win32_common.h"

#if __has_include(<afunix.h>)
#    include <afunix.h>
#else
struct sockaddr_un {
    ADDRESS_FAMILY sun_family; /* AF_UNIX */
    char sun_path[108];
};
#endif

#ifndef UNIX_PATH_MAX
#    define UNIX_PATH_MAX 108
#endif

#endif /* OPAL_WIN32_SYS_UN_H */
