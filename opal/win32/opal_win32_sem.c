/*
 * Copyright (c) 2026      Amazon.com, Inc. or its affiliates.
 *                         All Rights reserved.
 * $COPYRIGHT$
 * $HEADER$
 * POSIX semaphore implementation for the native Windows build.
 * Unnamed semaphores use a mutex+condvar+count (sem_init lives in
 * process-private space); named ones use Win32 named semaphores.
 */

#include "opal_config.h"

#ifdef _WIN32

#    include "opal/win32/opal_win32.h"

#    undef sem_init
#    undef sem_destroy
#    undef sem_wait
#    undef sem_trywait
#    undef sem_timedwait
#    undef sem_post
#    undef sem_getvalue
#    undef sem_open
#    undef sem_close
#    undef sem_unlink

OPAL_WIN32_DECLSPEC int sem_init(sem_t *sem, int pshared, unsigned int value)
{
    (void) pshared;
    InitializeCriticalSection(&sem->lock);
    InitializeConditionVariable(&sem->cond);
    sem->count = (LONG) value;
    sem->max = SEM_VALUE_MAX;
    sem->handle = NULL;
    sem->named = 0;
    return 0;
}

OPAL_WIN32_DECLSPEC int sem_destroy(sem_t *sem)
{
    if (sem->named && sem->handle) {
        CloseHandle(sem->handle);
    }
    DeleteCriticalSection(&sem->lock);
    return 0;
}

OPAL_WIN32_DECLSPEC int sem_wait(sem_t *sem)
{
    if (sem->named && sem->handle) {
        DWORD rc = WaitForSingleObject(sem->handle, INFINITE);
        return (WAIT_OBJECT_0 == rc) ? 0 : -1;
    }
    EnterCriticalSection(&sem->lock);
    while (sem->count <= 0) {
        SleepConditionVariableCS(&sem->cond, &sem->lock, INFINITE);
    }
    sem->count--;
    LeaveCriticalSection(&sem->lock);
    return 0;
}

OPAL_WIN32_DECLSPEC int sem_trywait(sem_t *sem)
{
    int ok = 0;
    if (sem->named && sem->handle) {
        DWORD rc = WaitForSingleObject(sem->handle, 0);
        return (WAIT_OBJECT_0 == rc) ? 0 : ((WAIT_TIMEOUT == rc) ? (errno = EAGAIN, -1)
                                                                : -1);
    }
    EnterCriticalSection(&sem->lock);
    if (sem->count > 0) {
        sem->count--;
        ok = 1;
    }
    LeaveCriticalSection(&sem->lock);
    if (!ok) {
        errno = EAGAIN;
        return -1;
    }
    return 0;
}

OPAL_WIN32_DECLSPEC int sem_timedwait(sem_t *sem, const struct timespec *abs_timeout)
{
    struct timespec now;
    long long rem;
    DWORD ms;
    BOOL rc;
    clock_gettime(CLOCK_REALTIME, &now);
    rem = ((LONGLONG) abs_timeout->tv_sec - now.tv_sec) * 1000000000LL
          + (abs_timeout->tv_nsec - now.tv_nsec);
    if (rem <= 0) {
        errno = ETIMEDOUT;
        return -1;
    }
    ms = (DWORD) (rem / 1000000LL + 1);
    if (sem->named && sem->handle) {
        DWORD r = WaitForSingleObject(sem->handle, ms);
        if (WAIT_OBJECT_0 == r) {
            return 0;
        }
        errno = (WAIT_TIMEOUT == r) ? ETIMEDOUT : EINVAL;
        return -1;
    }
    EnterCriticalSection(&sem->lock);
    while (sem->count <= 0) {
        rc = SleepConditionVariableCS(&sem->cond, &sem->lock, ms);
        if (!rc) {
            LeaveCriticalSection(&sem->lock);
            errno = (GetLastError() == ERROR_TIMEOUT) ? ETIMEDOUT : EINVAL;
            return -1;
        }
    }
    sem->count--;
    LeaveCriticalSection(&sem->lock);
    return 0;
}

OPAL_WIN32_DECLSPEC int sem_post(sem_t *sem)
{
    if (sem->named && sem->handle) {
        return ReleaseSemaphore(sem->handle, 1, NULL) ? 0 : -1;
    }
    EnterCriticalSection(&sem->lock);
    if (sem->count < sem->max) {
        sem->count++;
        WakeConditionVariable(&sem->cond);
    }
    LeaveCriticalSection(&sem->lock);
    return 0;
}

OPAL_WIN32_DECLSPEC int sem_getvalue(sem_t *sem, int *sval)
{
    if (sem->named && sem->handle) {
        *sval = 0;
        return 0;
    }
    EnterCriticalSection(&sem->lock);
    *sval = (int) sem->count;
    LeaveCriticalSection(&sem->lock);
    return 0;
}

OPAL_WIN32_DECLSPEC sem_t *sem_open(const char *name, int oflag, ...)
{
    sem_t *sem;
    char wname[MAX_PATH + 16];
    HANDLE h;
    int created = 0;
    unsigned initial = 0;
    va_list ap;

    if (oflag & O_CREAT) {
        va_start(ap, oflag);
        (void) va_arg(ap, int); /* mode */
        initial = va_arg(ap, unsigned int);
        va_end(ap);
    }
    snprintf(wname, sizeof(wname), "Local\\ompi_sem_%s", name);
    h = CreateSemaphoreA(NULL, (LONG) initial, SEM_VALUE_MAX, wname);
    if (NULL == h) {
        errno = ENOSPC;
        return SEM_FAILED;
    }
    if ((oflag & (O_CREAT | O_EXCL)) == (O_CREAT | O_EXCL)) {
        if (GetLastError() == ERROR_ALREADY_EXISTS) {
            CloseHandle(h);
            errno = EEXIST;
            return SEM_FAILED;
        }
    }
    (void) created;
    sem = calloc(1, sizeof(*sem));
    if (NULL == sem) {
        CloseHandle(h);
        errno = ENOMEM;
        return SEM_FAILED;
    }
    sem->handle = h;
    sem->named = 1;
    InitializeCriticalSection(&sem->lock);
    InitializeConditionVariable(&sem->cond);
    return sem;
}

OPAL_WIN32_DECLSPEC int sem_close(sem_t *sem)
{
    if (NULL == sem) {
        return -1;
    }
    if (sem->handle) {
        CloseHandle(sem->handle);
    }
    DeleteCriticalSection(&sem->lock);
    free(sem);
    return 0;
}

OPAL_WIN32_DECLSPEC int sem_unlink(const char *name)
{
    (void) name;
    return 0;
}

#endif /* _WIN32 */
