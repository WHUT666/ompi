/*
 * Copyright (c) 2026      Amazon.com, Inc. or its affiliates.
 *                         All Rights reserved.
 * $COPYRIGHT$
 * $HEADER$
 * POSIX <sys/param.h> replacement for the native Windows build.
 */
#ifndef OPAL_WIN32_SYS_PARAM_H
#define OPAL_WIN32_SYS_PARAM_H

#include "opal_win32_common.h"
#include <limits.h>

#ifndef MAXPATHLEN
#    define MAXPATHLEN MAX_PATH
#endif
#ifndef PATH_MAX
#    define PATH_MAX MAX_PATH
#endif
#ifndef MAXHOSTNAMELEN
#    define MAXHOSTNAMELEN 256
#endif
#ifndef HOST_NAME_MAX
#    define HOST_NAME_MAX 256
#endif
#ifndef MAXSYMLINKS
#    define MAXSYMLINKS 8
#endif
#ifndef MAXNAMELEN
#    define MAXNAMELEN 255
#endif
#ifndef NAME_MAX
#    define NAME_MAX 255
#endif
#ifndef MAXNAMLEN
#    define MAXNAMLEN NAME_MAX
#endif
#ifndef LINE_MAX
#    define LINE_MAX 2048
#endif
#ifndef LOGIN_NAME_MAX
#    define LOGIN_NAME_MAX 256
#endif
#ifndef NCARGS
#    define NCARGS 32768
#endif
#ifndef NGROUPS_MAX
#    define NGROUPS_MAX 16
#endif
#ifndef NOFILE
#    define NOFILE 512
#endif
#ifndef CANBSIZ
#    define CANBSIZ 255
#endif
#ifndef MAXINTERP
#    define MAXINTERP 64
#endif
#ifndef DEV_BSIZE
#    define DEV_BSIZE 512
#endif
#ifndef BLKSIZE
#    define BLKSIZE 4096
#endif

#ifndef MIN
#    define MIN(a, b) (((a) < (b)) ? (a) : (b))
#endif
#ifndef MAX
#    define MAX(a, b) (((a) > (b)) ? (a) : (b))
#endif
#ifndef howmany
#    define howmany(x, y) (((x) + ((y) - 1)) / (y))
#endif
#ifndef roundup
#    define roundup(x, y) ((((x) + ((y) - 1)) / (y)) * (y))
#endif
#ifndef powerof2
#    define powerof2(x) ((((x) - 1) & (x)) == 0)
#endif
#ifndef roundup2
#    define roundup2(x, y) (((x) + ((y) - 1)) & (~((y) - 1)))
#endif

#ifndef BYTE_ORDER
#    define LITTLE_ENDIAN 1234
#    define BIG_ENDIAN    4321
#    define PDP_ENDIAN    3412
#    define BYTE_ORDER    LITTLE_ENDIAN
#endif

#ifndef HZ
#    define HZ 100
#endif

#endif /* OPAL_WIN32_SYS_PARAM_H */
