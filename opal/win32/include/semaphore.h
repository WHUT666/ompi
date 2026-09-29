/*
 * Copyright (c) 2026      Amazon.com, Inc. or its affiliates.
 *                         All Rights reserved.
 * $COPYRIGHT$
 * $HEADER$
 * POSIX <semaphore.h> replacement for the native Windows build.
 */
#ifndef OPAL_WIN32_SEMAPHORE_H
#define OPAL_WIN32_SEMAPHORE_H

#include "opal_win32_common.h"
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct sem_t {
    HANDLE handle;           /* unnamed: Win32 semaphore; NULL if malloc'd failed */
    volatile LONG count;
    volatile LONG max;
    CRITICAL_SECTION lock;
    CONDITION_VARIABLE cond;
    int named;               /* nonzero when backed by a named object */
} sem_t;

#define SEM_FAILED ((sem_t *) 0)
#define SEM_VALUE_MAX INT_MAX

OPAL_WIN32_DECLSPEC int sem_init(sem_t *sem, int pshared, unsigned int value);
OPAL_WIN32_DECLSPEC int sem_destroy(sem_t *sem);
OPAL_WIN32_DECLSPEC int sem_wait(sem_t *sem);
OPAL_WIN32_DECLSPEC int sem_trywait(sem_t *sem);
OPAL_WIN32_DECLSPEC int sem_timedwait(sem_t *sem, const struct timespec *abs_timeout);
OPAL_WIN32_DECLSPEC int sem_post(sem_t *sem);
OPAL_WIN32_DECLSPEC int sem_getvalue(sem_t *sem, int *sval);
sem_t *sem_open(const char *name, int oflag, ...);
OPAL_WIN32_DECLSPEC int sem_close(sem_t *sem);
OPAL_WIN32_DECLSPEC int sem_unlink(const char *name);

#ifdef __cplusplus
}
#endif

#endif /* OPAL_WIN32_SEMAPHORE_H */
