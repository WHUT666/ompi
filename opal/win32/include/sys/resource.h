/*
 * Copyright (c) 2026      Amazon.com, Inc. or its affiliates.
 *                         All Rights reserved.
 * $COPYRIGHT$
 * $HEADER$
 * POSIX <sys/resource.h> replacement for the native Windows build.
 * rlimits are largely advisory stubs (Windows has no RLIMIT_*); the
 * reported values are generous defaults so sizing code works.
 */
#ifndef OPAL_WIN32_SYS_RESOURCE_H
#define OPAL_WIN32_SYS_RESOURCE_H

#include "opal_win32_common.h"
#include <sys/time.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned long long rlim_t;
#define RLIM_INFINITY   (~0ULL)
#define RLIM_SAVED_MAX  RLIM_INFINITY
#define RLIM_SAVED_CUR  RLIM_INFINITY

#define RLIMIT_CPU      0
#define RLIMIT_FSIZE    1
#define RLIMIT_DATA     2
#define RLIMIT_STACK    3
#define RLIMIT_CORE     4
#define RLIMIT_RSS      5
#define RLIMIT_NPROC    6
#define RLIMIT_NOFILE   7
#define RLIMIT_MEMLOCK  8
#define RLIMIT_AS       9
#define RLIMIT_LOCKS    10
#define RLIMIT_SIGPENDING 11
#define RLIMIT_MSGQUEUE 12
#define RLIMIT_NICE     13
#define RLIMIT_RTPRIO   14
#define RLIMIT_RTTIME   15
#define RLIMIT_NLIMITS  16
#define RLIM_NLIMITS    RLIMIT_NLIMITS

struct rlimit {
    rlim_t rlim_cur;
    rlim_t rlim_max;
};

#define RUSAGE_SELF     0
#define RUSAGE_CHILDREN (-1)
#define RUSAGE_THREAD   1
#define RUSAGE_LWP      RUSAGE_THREAD

struct rusage {
    struct timeval ru_utime;
    struct timeval ru_stime;
    long ru_maxrss;
    long ru_ixrss;
    long ru_idrss;
    long ru_isrss;
    long ru_minflt;
    long ru_majflt;
    long ru_nswap;
    long ru_inblock;
    long ru_oublock;
    long ru_msgsnd;
    long ru_msgrcv;
    long ru_nsignals;
    long ru_nvcsw;
    long ru_nivcsw;
};

#define PRIO_PROCESS 0
#define PRIO_PGRP    1
#define PRIO_USER    2

OPAL_WIN32_DECLSPEC int opal_win32_getrlimit(int resource, struct rlimit *rlim);
OPAL_WIN32_DECLSPEC int opal_win32_setrlimit(int resource, const struct rlimit *rlim);
OPAL_WIN32_DECLSPEC int opal_win32_getrusage(int who, struct rusage *usage);
OPAL_WIN32_DECLSPEC int opal_win32_getpriority(int which, int who);
OPAL_WIN32_DECLSPEC int opal_win32_setpriority(int which, int who, int prio);

#define getrlimit   opal_win32_getrlimit
#define setrlimit   opal_win32_setrlimit
#define getrusage   opal_win32_getrusage
#define getpriority opal_win32_getpriority
#define setpriority opal_win32_setpriority

#ifdef __cplusplus
}
#endif

#endif /* OPAL_WIN32_SYS_RESOURCE_H */
