/*
 * Copyright (c) 2026      Amazon.com, Inc. or its affiliates.
 *                         All Rights reserved.
 * $COPYRIGHT$
 * $HEADER$
 * POSIX <sys/time.h> replacement for the native Windows build.
 * struct timeval comes from Winsock2; this adds timersub-style macros
 * and gettimeofday()/setitimer() declarations.
 */
#ifndef OPAL_WIN32_SYS_TIME_H
#define OPAL_WIN32_SYS_TIME_H

#include "opal_win32_common.h"
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _WINSOCK2API_
/* winsock2.h already provides struct timeval when included via
 * opal_win32_common.h; if a TU somehow got here without it, provide it */
struct timeval {
    long tv_sec;
    long tv_usec;
};
#endif

#ifndef _OPAL_WIN32_TIMEZONE_DEFINED
#    define _OPAL_WIN32_TIMEZONE_DEFINED
struct timezone {
    int tz_minuteswest;
    int tz_dsttime;
};
#endif

OPAL_WIN32_DECLSPEC int opal_win32_gettimeofday(struct timeval *tv, void *tz);
#define gettimeofday opal_win32_gettimeofday

#ifndef timerisset
#    define timerisset(tvp) ((tvp)->tv_sec || (tvp)->tv_usec)
#endif
#ifndef timerclear
#    define timerclear(tvp) ((tvp)->tv_sec = (tvp)->tv_usec = 0)
#endif
#ifndef timercmp
#    define timercmp(a, b, CMP)                                               \
        (((a)->tv_sec == (b)->tv_sec) ? ((a)->tv_usec CMP(b)->tv_usec)        \
                                      : ((a)->tv_sec CMP(b)->tv_sec))
#endif
#ifndef timeradd
#    define timeradd(a, b, result)                                            \
    do {                                                                      \
        (result)->tv_sec = (a)->tv_sec + (b)->tv_sec;                         \
        (result)->tv_usec = (a)->tv_usec + (b)->tv_usec;                      \
        if ((result)->tv_usec >= 1000000) {                                   \
            ++(result)->tv_sec;                                               \
            (result)->tv_usec -= 1000000;                                     \
        }                                                                     \
    } while (0)
#endif
#ifndef timersub
#    define timersub(a, b, result)                                            \
    do {                                                                      \
        (result)->tv_sec = (a)->tv_sec - (b)->tv_sec;                         \
        (result)->tv_usec = (a)->tv_usec - (b)->tv_usec;                      \
        if ((result)->tv_usec < 0) {                                          \
            --(result)->tv_sec;                                               \
            (result)->tv_usec += 1000000;                                     \
        }                                                                     \
    } while (0)
#endif
#define timerclear_frac(tvp) timerclear(tvp)

/* interval timers -- stubs; the code base only uses them for optional
 * profiling/signal paths that we disable on Windows */
struct itimerval {
    struct timeval it_interval;
    struct timeval it_value;
};
#define ITIMER_REAL    0
#define ITIMER_VIRTUAL 1
#define ITIMER_PROF    2
OPAL_WIN32_DECLSPEC int  opal_win32_getitimer(int which, struct itimerval *value);
OPAL_WIN32_DECLSPEC int  opal_win32_setitimer(int which, const struct itimerval *value,
                          struct itimerval *ovalue);
#define getitimer opal_win32_getitimer
#define setitimer opal_win32_setitimer

OPAL_WIN32_DECLSPEC int opal_win32_utimes(const char *path, const struct timeval times[2]);
#define utimes opal_win32_utimes
struct timespec; /* forward */
OPAL_WIN32_DECLSPEC int opal_win32_utimensat(int fd, const char *path, const struct timespec *times,
                         int flag);
#define utimensat opal_win32_utimensat

#ifdef __cplusplus
}
#endif

#endif /* OPAL_WIN32_SYS_TIME_H */
