/*
 * Copyright (c) 2026      Amazon.com, Inc. or its affiliates.
 *                         All Rights reserved.
 * $COPYRIGHT$
 * $HEADER$
 * POSIX <sched.h> replacement for the native Windows build.
 */
#ifndef OPAL_WIN32_SCHED_H
#define OPAL_WIN32_SCHED_H

#include "opal_win32_common.h"
#include <sys/types.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _SCHED_PARAM_DEFINED
#    define _SCHED_PARAM_DEFINED
struct sched_param {
    int sched_priority;
};
#endif

/* GNU-style cpu_set_t covering 512 logical processors */
#ifndef _CPU_SET_T_DEFINED
#    define _CPU_SET_T_DEFINED
typedef struct {
    unsigned long long bits[8];
} cpu_set_t;
#endif
#ifndef CPU_SETSIZE
#    define CPU_SETSIZE 512
#endif
#ifndef CPU_ZERO
#    define CPU_ZERO(s)   memset((s), 0, sizeof(cpu_set_t))
#endif
#ifndef CPU_SET
#    define CPU_SET(n, s) ((s)->bits[(n) >> 6] |= (1ULL << ((n) & 63)))
#endif
#ifndef CPU_CLR
#    define CPU_CLR(n, s) ((s)->bits[(n) >> 6] &= ~(1ULL << ((n) & 63)))
#endif
#ifndef CPU_ISSET
#    define CPU_ISSET(n, s) \
        (((s)->bits[(n) >> 6] >> ((n) & 63)) & 1ULL)
#endif
OPAL_WIN32_DECLSPEC int opal_win32_cpu_count(const cpu_set_t *s);
#ifndef CPU_COUNT
#    define CPU_COUNT(s) opal_win32_cpu_count(s)
#endif

#ifndef SCHED_OTHER
#    define SCHED_OTHER 0
#endif
#ifndef SCHED_FIFO
#    define SCHED_FIFO 1
#endif
#ifndef SCHED_RR
#    define SCHED_RR 2
#endif

static inline int sched_yield(void)
{
    SwitchToThread();
    return 0;
}

OPAL_WIN32_DECLSPEC int opal_win32_sched_get_priority_max(int policy);
OPAL_WIN32_DECLSPEC int opal_win32_sched_get_priority_min(int policy);
OPAL_WIN32_DECLSPEC int opal_win32_sched_setscheduler(pid_t pid, int policy, const struct sched_param *p);
OPAL_WIN32_DECLSPEC int opal_win32_sched_getscheduler(pid_t pid);
OPAL_WIN32_DECLSPEC int opal_win32_sched_getparam(pid_t pid, struct sched_param *p);
OPAL_WIN32_DECLSPEC int opal_win32_sched_setparam(pid_t pid, const struct sched_param *p);
OPAL_WIN32_DECLSPEC int opal_win32_sched_setaffinity(pid_t pid, size_t size, const cpu_set_t *set);
OPAL_WIN32_DECLSPEC int opal_win32_sched_getaffinity(pid_t pid, size_t size, cpu_set_t *set);
OPAL_WIN32_DECLSPEC int opal_win32_sched_rr_get_interval(pid_t pid, struct timespec *t);

#define sched_get_priority_max opal_win32_sched_get_priority_max
#define sched_get_priority_min opal_win32_sched_get_priority_min
#define sched_setscheduler     opal_win32_sched_setscheduler
#define sched_getscheduler     opal_win32_sched_getscheduler
#define sched_getparam         opal_win32_sched_getparam
#define sched_setparam         opal_win32_sched_setparam
#define sched_setaffinity      opal_win32_sched_setaffinity
#define sched_getaffinity      opal_win32_sched_getaffinity
#define sched_rr_get_interval  opal_win32_sched_rr_get_interval

#define CPU_ALLOC(n) ((cpu_set_t *) calloc(1, sizeof(cpu_set_t)))
#define CPU_FREE(s)  free(s)

#ifdef __cplusplus
}
#endif

#endif /* OPAL_WIN32_SCHED_H */
