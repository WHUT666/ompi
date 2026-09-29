/*
 * Copyright (c) 2026      Amazon.com, Inc. or its affiliates.
 *                         All Rights reserved.
 * $COPYRIGHT$
 * $HEADER$
 * POSIX <sys/times.h> replacement for the native Windows build.
 */
#ifndef OPAL_WIN32_SYS_TIMES_H
#define OPAL_WIN32_SYS_TIMES_H

#include "opal_win32_common.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef long long clock_t_ticks;
struct tms {
    clock_t tms_utime;
    clock_t tms_stime;
    clock_t tms_cutime;
    clock_t tms_cstime;
};

OPAL_WIN32_DECLSPEC clock_t opal_win32_times(struct tms *buf);
#define times opal_win32_times

#ifdef __cplusplus
}
#endif

#endif /* OPAL_WIN32_SYS_TIMES_H */
