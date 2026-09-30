/*
 * Copyright (c) 2026      Amazon.com, Inc. or its affiliates.
 *                         All Rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 *
 * POSIX <pthread.h> replacement for the native Windows (MSVC) build
 * of Open MPI.  Implements the subset of pthreads that OPAL, PMIx and
 * PRRTE use, on top of Win32 primitives.  Out-of-line parts live in
 * opal/win32/opal_win32_pthread.c.
 */

#ifndef OPAL_WIN32_PTHREAD_H
#define OPAL_WIN32_PTHREAD_H

#include "opal_win32_common.h"
#include <time.h>
#include <sched.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ------------------------------------------------------------------ */
/* Threads                                                             */
/* ------------------------------------------------------------------ */

typedef struct opal_win32_thread {
    HANDLE handle;      /* Win32 thread handle (NULL for foreign threads) */
    DWORD  id;          /* Win32 thread id */
    void *(*start)(void *);
    void *arg;
    void *ret;
    volatile LONG detached;
    volatile LONG exited;
    int      cancel_requested;
} *pthread_t;

typedef struct pthread_attr_t {
    size_t stacksize;
    int    detachstate;
    int    inheritsched;
    int    schedpolicy;
    struct sched_param schedparam;
    void  *stackaddr;
    int    scope;
    int    guardsize;
} pthread_attr_t;

#define PTHREAD_CREATE_JOINABLE 0
#define PTHREAD_CREATE_DETACHED 1

#define PTHREAD_INHERIT_SCHED  0
#define PTHREAD_EXPLICIT_SCHED 1

#define PTHREAD_SCOPE_SYSTEM   0
#define PTHREAD_SCOPE_PROCESS  1

#define SCHED_OTHER   0
#define SCHED_FIFO    1
#define SCHED_RR      2

#define PTHREAD_PRIO_NONE     0
#define PTHREAD_PRIO_INHERIT  1
#define PTHREAD_PRIO_PROTECT  2

OPAL_WIN32_DECLSPEC int pthread_create(pthread_t *thread, const pthread_attr_t *attr,
                    void *(*start_routine)(void *), void *arg);
OPAL_WIN32_DECLSPEC int pthread_join(pthread_t thread, void **retval);
OPAL_WIN32_DECLSPEC int pthread_detach(pthread_t thread);
OPAL_WIN32_DECLSPEC void pthread_exit(void *retval);
OPAL_WIN32_DECLSPEC pthread_t pthread_self(void);
OPAL_WIN32_DECLSPEC int pthread_equal(pthread_t t1, pthread_t t2);
OPAL_WIN32_DECLSPEC int pthread_kill(pthread_t thread, int sig);
OPAL_WIN32_DECLSPEC int pthread_cancel(pthread_t thread);
OPAL_WIN32_DECLSPEC int pthread_setcancelstate(int state, int *oldstate);
OPAL_WIN32_DECLSPEC int pthread_setcanceltype(int type, int *oldtype);
OPAL_WIN32_DECLSPEC void pthread_testcancel(void);
OPAL_WIN32_DECLSPEC int pthread_atfork(void (*prepare)(void), void (*parent)(void),
                    void (*child)(void));
OPAL_WIN32_DECLSPEC int pthread_setname_np(pthread_t thread, const char *name);
OPAL_WIN32_DECLSPEC int pthread_getname_np(pthread_t thread, char *name, size_t len);

OPAL_WIN32_DECLSPEC int pthread_attr_init(pthread_attr_t *attr);
OPAL_WIN32_DECLSPEC int pthread_attr_destroy(pthread_attr_t *attr);
OPAL_WIN32_DECLSPEC int pthread_attr_setdetachstate(pthread_attr_t *attr, int detachstate);
OPAL_WIN32_DECLSPEC int pthread_attr_getdetachstate(const pthread_attr_t *attr, int *detachstate);
OPAL_WIN32_DECLSPEC int pthread_attr_setstacksize(pthread_attr_t *attr, size_t stacksize);
OPAL_WIN32_DECLSPEC int pthread_attr_getstacksize(const pthread_attr_t *attr, size_t *stacksize);
OPAL_WIN32_DECLSPEC int pthread_attr_setguardsize(pthread_attr_t *attr, size_t guardsize);
OPAL_WIN32_DECLSPEC int pthread_attr_getguardsize(const pthread_attr_t *attr, size_t *guardsize);
OPAL_WIN32_DECLSPEC int pthread_attr_setscope(pthread_attr_t *attr, int scope);
OPAL_WIN32_DECLSPEC int pthread_attr_getscope(const pthread_attr_t *attr, int *scope);
OPAL_WIN32_DECLSPEC int pthread_attr_setschedparam(pthread_attr_t *attr, const struct sched_param *param);
OPAL_WIN32_DECLSPEC int pthread_attr_getschedparam(const pthread_attr_t *attr, struct sched_param *param);
OPAL_WIN32_DECLSPEC int pthread_attr_setschedpolicy(pthread_attr_t *attr, int policy);
OPAL_WIN32_DECLSPEC int pthread_attr_getschedpolicy(const pthread_attr_t *attr, int *policy);
OPAL_WIN32_DECLSPEC int pthread_attr_setinheritsched(pthread_attr_t *attr, int inheritsched);
OPAL_WIN32_DECLSPEC int pthread_attr_getinheritsched(const pthread_attr_t *attr, int *inheritsched);
OPAL_WIN32_DECLSPEC int pthread_attr_setstackaddr(pthread_attr_t *attr, void *stackaddr);
OPAL_WIN32_DECLSPEC int pthread_attr_getstackaddr(const pthread_attr_t *attr, void **stackaddr);
OPAL_WIN32_DECLSPEC int pthread_attr_setstack(pthread_attr_t *attr, void *stackaddr, size_t stacksize);
OPAL_WIN32_DECLSPEC int pthread_attr_getstack(const pthread_attr_t *attr, void **stackaddr, size_t *stacksize);

OPAL_WIN32_DECLSPEC int pthread_getschedparam(pthread_t thread, int *policy, struct sched_param *param);
OPAL_WIN32_DECLSPEC int pthread_setschedparam(pthread_t thread, int policy, const struct sched_param *param);
OPAL_WIN32_DECLSPEC int pthread_setschedprio(pthread_t thread, int prio);
OPAL_WIN32_DECLSPEC int pthread_getconcurrency(void);
OPAL_WIN32_DECLSPEC int pthread_setconcurrency(int level);

#define PTHREAD_CANCEL_ENABLE    0
#define PTHREAD_CANCEL_DISABLE   1
#define PTHREAD_CANCEL_DEFERRED  0
#define PTHREAD_CANCEL_ASYNCHRONOUS 1
#define PTHREAD_CANCELED         ((void *) -1)

/* ------------------------------------------------------------------ */
/* Mutexes                                                             */
/*                                                                     */
/* A CRITICAL_SECTION cannot be statically initialized, so we keep a   */
/* state word: 0 = needs init, 1 = initializing, 2 = ready.  The fast  */
/* path is a single load+predictable branch.                           */
/* ------------------------------------------------------------------ */

typedef struct pthread_mutex_t {
    volatile LONG      _state;
    int                _kind;   /* PTHREAD_MUTEX_* | _MUTEX_PSHARED */
    CRITICAL_SECTION   _cs;
    /* Process-shared fast path, valid when _kind has _MUTEX_PSHARED set.
     * The object then lives in shared memory (e.g. an mmap'd segment);
     * a CRITICAL_SECTION is process-private and cannot be used there,
     * so locking is an interlocked spin on _sh_lock instead.  The two
     * extra words also sit in the shared segment. */
    volatile LONG      _sh_lock;
    volatile LONG      _sh_pad;
} pthread_mutex_t;

/* internal flag OR'd into _kind for PTHREAD_PROCESS_SHARED mutexes */
#define _MUTEX_PSHARED 0x10000

typedef struct pthread_mutexattr_t {
    int type;
    int pshared;
    int robust;
    int protocol;
    int prioceiling;
} pthread_mutexattr_t;

#define PTHREAD_MUTEX_NORMAL           0
#define PTHREAD_MUTEX_ERRORCHECK       1
#define PTHREAD_MUTEX_RECURSIVE        2
#define PTHREAD_MUTEX_DEFAULT          PTHREAD_MUTEX_NORMAL
#define PTHREAD_MUTEX_ERRORCHECK_NP    PTHREAD_MUTEX_ERRORCHECK
#define PTHREAD_MUTEX_RECURSIVE_NP     PTHREAD_MUTEX_RECURSIVE
#define PTHREAD_PROCESS_PRIVATE        0
#define PTHREAD_PROCESS_SHARED         1
#define PTHREAD_MUTEX_STALLED          0
#define PTHREAD_MUTEX_ROBUST           1

#define PTHREAD_MUTEX_INITIALIZER {0, PTHREAD_MUTEX_NORMAL, {0}, 0, 0}
#define PTHREAD_RECURSIVE_MUTEX_INITIALIZER {0, PTHREAD_MUTEX_RECURSIVE, {0}, 0, 0}
#define PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP PTHREAD_RECURSIVE_MUTEX_INITIALIZER
#define PTHREAD_ERRORCHECK_MUTEX_INITIALIZER_NP {0, PTHREAD_MUTEX_ERRORCHECK, {0}, 0, 0}

static inline int opal_win32_mutex_lazy_init(pthread_mutex_t *m)
{
    LONG st = InterlockedCompareExchange(&m->_state, 1, 0);
    if (0 == st) {
        InitializeCriticalSection(&m->_cs);
        InterlockedExchange(&m->_state, 2);
        return 0;
    }
    while (1 == st) {
        YieldProcessor();
        st = m->_state;
    }
    return 0;
}

static inline int opal_win32_mutex_pshared_lock(pthread_mutex_t *m)
{
    unsigned spins = 0;
    while (InterlockedCompareExchange(&m->_sh_lock, 1, 0)) {
        if (++spins < 64) {
            YieldProcessor();
        } else if (spins < 1024) {
            SwitchToThread();
        } else {
            Sleep(1);
        }
    }
    return 0;
}

static inline int pthread_mutex_lock(pthread_mutex_t *m)
{
    if (2 != m->_state) {
        opal_win32_mutex_lazy_init(m);
    }
    if (m->_kind & _MUTEX_PSHARED) {
        return opal_win32_mutex_pshared_lock(m);
    }
    EnterCriticalSection(&m->_cs);
    return 0;
}

static inline int pthread_mutex_trylock(pthread_mutex_t *m)
{
    if (2 != m->_state) {
        opal_win32_mutex_lazy_init(m);
    }
    if (m->_kind & _MUTEX_PSHARED) {
        return InterlockedCompareExchange(&m->_sh_lock, 1, 0) ? EBUSY : 0;
    }
    return TryEnterCriticalSection(&m->_cs) ? 0 : EBUSY;
}

static inline int pthread_mutex_unlock(pthread_mutex_t *m)
{
    if (m->_kind & _MUTEX_PSHARED) {
        InterlockedExchange(&m->_sh_lock, 0);
        return 0;
    }
    LeaveCriticalSection(&m->_cs);
    return 0;
}

OPAL_WIN32_DECLSPEC int pthread_mutex_init(pthread_mutex_t *m, const pthread_mutexattr_t *attr);
OPAL_WIN32_DECLSPEC int pthread_mutex_destroy(pthread_mutex_t *m);
OPAL_WIN32_DECLSPEC int pthread_mutex_timedlock(pthread_mutex_t *m, const struct timespec *abstime);
OPAL_WIN32_DECLSPEC int pthread_mutex_getprioceiling(const pthread_mutex_t *m, int *ceiling);
OPAL_WIN32_DECLSPEC int pthread_mutex_setprioceiling(pthread_mutex_t *m, int ceiling, int *old);
OPAL_WIN32_DECLSPEC int pthread_mutex_consistent(pthread_mutex_t *m);

OPAL_WIN32_DECLSPEC int pthread_mutexattr_init(pthread_mutexattr_t *attr);
OPAL_WIN32_DECLSPEC int pthread_mutexattr_destroy(pthread_mutexattr_t *attr);
OPAL_WIN32_DECLSPEC int pthread_mutexattr_settype(pthread_mutexattr_t *attr, int type);
OPAL_WIN32_DECLSPEC int pthread_mutexattr_gettype(const pthread_mutexattr_t *attr, int *type);
OPAL_WIN32_DECLSPEC int pthread_mutexattr_setpshared(pthread_mutexattr_t *attr, int pshared);
OPAL_WIN32_DECLSPEC int pthread_mutexattr_getpshared(const pthread_mutexattr_t *attr, int *pshared);
OPAL_WIN32_DECLSPEC int pthread_mutexattr_setrobust(pthread_mutexattr_t *attr, int robust);
OPAL_WIN32_DECLSPEC int pthread_mutexattr_getrobust(const pthread_mutexattr_t *attr, int *robust);
OPAL_WIN32_DECLSPEC int pthread_mutexattr_setprotocol(pthread_mutexattr_t *attr, int protocol);
OPAL_WIN32_DECLSPEC int pthread_mutexattr_getprotocol(const pthread_mutexattr_t *attr, int *protocol);
OPAL_WIN32_DECLSPEC int pthread_mutexattr_setprioceiling(pthread_mutexattr_t *attr, int prioceiling);
OPAL_WIN32_DECLSPEC int pthread_mutexattr_getprioceiling(const pthread_mutexattr_t *attr, int *prioceiling);

/* ------------------------------------------------------------------ */
/* Condition variables                                                 */
/* ------------------------------------------------------------------ */

/* CONDITION_VARIABLE is process-private, so a process-shared condvar is
 * emulated with a generation counter living in shared memory: a waiter
 * samples _sh_gen under the mutex, releases it, then yields until the
 * generation changes; signal/broadcast bump the generation (waking all
 * waiters, which re-check their predicate -- legal POSIX behaviour). */
typedef struct pthread_cond_t {
    CONDITION_VARIABLE _cv;
    volatile LONG      _sh_kind;   /* nonzero => process-shared */
    volatile LONG      _sh_gen;
} pthread_cond_t;

typedef struct pthread_condattr_t {
    int pshared;
    int clock_id;
} pthread_condattr_t;

#define PTHREAD_COND_INITIALIZER {CONDITION_VARIABLE_INIT, 0, 0}

OPAL_WIN32_DECLSPEC int pthread_cond_init(pthread_cond_t *cond, const pthread_condattr_t *attr);
OPAL_WIN32_DECLSPEC int pthread_cond_destroy(pthread_cond_t *cond);

static inline int pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex)
{
    if (2 != mutex->_state) {
        opal_win32_mutex_lazy_init(mutex);
    }
    if (cond->_sh_kind) {
        LONG gen = cond->_sh_gen;
        unsigned spins = 0;
        pthread_mutex_unlock(mutex);
        while (cond->_sh_gen == gen) {
            if (++spins < 64) {
                YieldProcessor();
            } else if (spins < 1024) {
                SwitchToThread();
            } else {
                Sleep(1);
            }
        }
        return pthread_mutex_lock(mutex);
    }
    return SleepConditionVariableCS(&cond->_cv, &mutex->_cs, INFINITE) ? 0 : EINVAL;
}

static inline int pthread_cond_signal(pthread_cond_t *cond)
{
    if (cond->_sh_kind) {
        InterlockedIncrement(&cond->_sh_gen);
        return 0;
    }
    WakeConditionVariable(&cond->_cv);
    return 0;
}

static inline int pthread_cond_broadcast(pthread_cond_t *cond)
{
    if (cond->_sh_kind) {
        InterlockedIncrement(&cond->_sh_gen);
        return 0;
    }
    WakeAllConditionVariable(&cond->_cv);
    return 0;
}

OPAL_WIN32_DECLSPEC int pthread_cond_timedwait(pthread_cond_t *cond, pthread_mutex_t *mutex,
                           const struct timespec *abstime);
OPAL_WIN32_DECLSPEC int pthread_condattr_init(pthread_condattr_t *attr);
OPAL_WIN32_DECLSPEC int pthread_condattr_destroy(pthread_condattr_t *attr);
OPAL_WIN32_DECLSPEC int pthread_condattr_setclock(pthread_condattr_t *attr, int clock_id);
OPAL_WIN32_DECLSPEC int pthread_condattr_getclock(const pthread_condattr_t *attr, int *clock_id);
OPAL_WIN32_DECLSPEC int pthread_condattr_setpshared(pthread_condattr_t *attr, int pshared);
OPAL_WIN32_DECLSPEC int pthread_condattr_getpshared(const pthread_condattr_t *attr, int *pshared);

/* ------------------------------------------------------------------ */
/* Reader/writer locks (slim RW locks)                                 */
/* ------------------------------------------------------------------ */

typedef struct pthread_rwlock_t {
    volatile LONG      _init;   /* lazy-init state word (0/1/2) */
    CRITICAL_SECTION   cs;
    CONDITION_VARIABLE cv_readers;
    CONDITION_VARIABLE cv_writers;
    volatile LONG      readers;          /* active readers */
    volatile LONG      writer_active;    /* a writer holds the lock */
    volatile LONG      writers_waiting;  /* blocked writers */
} pthread_rwlock_t;

typedef struct pthread_rwlockattr_t {
    int pshared;
} pthread_rwlockattr_t;

#define PTHREAD_RWLOCK_INITIALIZER \
    {                              \
        0, {0}, {0}, {0}, 0, 0, 0  \
    }

OPAL_WIN32_DECLSPEC int pthread_rwlock_init(pthread_rwlock_t *l, const pthread_rwlockattr_t *a);
OPAL_WIN32_DECLSPEC int pthread_rwlock_destroy(pthread_rwlock_t *l);
OPAL_WIN32_DECLSPEC int pthread_rwlock_rdlock(pthread_rwlock_t *l);
OPAL_WIN32_DECLSPEC int pthread_rwlock_wrlock(pthread_rwlock_t *l);
OPAL_WIN32_DECLSPEC int pthread_rwlock_tryrdlock(pthread_rwlock_t *l);
OPAL_WIN32_DECLSPEC int pthread_rwlock_trywrlock(pthread_rwlock_t *l);
OPAL_WIN32_DECLSPEC int pthread_rwlock_unlock(pthread_rwlock_t *l);

OPAL_WIN32_DECLSPEC int pthread_rwlockattr_init(pthread_rwlockattr_t *attr);
OPAL_WIN32_DECLSPEC int pthread_rwlockattr_destroy(pthread_rwlockattr_t *attr);
OPAL_WIN32_DECLSPEC int pthread_rwlockattr_setpshared(pthread_rwlockattr_t *attr, int pshared);
OPAL_WIN32_DECLSPEC int pthread_rwlockattr_getpshared(const pthread_rwlockattr_t *attr, int *pshared);

/* ------------------------------------------------------------------ */
/* Once                                                                */
/* ------------------------------------------------------------------ */

typedef INIT_ONCE pthread_once_t;
#define PTHREAD_ONCE_INIT INIT_ONCE_STATIC_INIT

OPAL_WIN32_DECLSPEC int pthread_once(pthread_once_t *once, void (*init_routine)(void));

/* ------------------------------------------------------------------ */
/* Thread-specific data (Fiber Local Storage)                          */
/* ------------------------------------------------------------------ */

typedef DWORD pthread_key_t;

OPAL_WIN32_DECLSPEC int pthread_key_create(pthread_key_t *key, void (*destructor)(void *));
OPAL_WIN32_DECLSPEC int pthread_key_delete(pthread_key_t key);
OPAL_WIN32_DECLSPEC int pthread_setspecific(pthread_key_t key, const void *value);
void *pthread_getspecific(pthread_key_t key);

/* ------------------------------------------------------------------ */
/* Spin locks                                                          */
/* ------------------------------------------------------------------ */

typedef volatile LONG pthread_spinlock_t;

static inline int pthread_spin_init(pthread_spinlock_t *l, int pshared)
{
    (void) pshared;
    *l = 0;
    return 0;
}

static inline int pthread_spin_destroy(pthread_spinlock_t *l)
{
    (void) l;
    return 0;
}

static inline int pthread_spin_lock(pthread_spinlock_t *l)
{
    while (InterlockedExchange(l, 1) != 0) {
        while (*l) {
            YieldProcessor();
        }
    }
    return 0;
}

static inline int pthread_spin_trylock(pthread_spinlock_t *l)
{
    return InterlockedExchange(l, 1) == 0 ? 0 : EBUSY;
}

static inline int pthread_spin_unlock(pthread_spinlock_t *l)
{
    InterlockedExchange(l, 0);
    return 0;
}

/* ------------------------------------------------------------------ */
/* Barriers                                                            */
/* ------------------------------------------------------------------ */

typedef struct pthread_barrier_t {
    volatile LONG      _init;  /* lazy-init state word */
    CRITICAL_SECTION   cs;
    CONDITION_VARIABLE cv;
    unsigned           count;  /* participants remaining this generation */
    unsigned           total;  /* participants per generation */
    unsigned           generation;
} pthread_barrier_t;

typedef struct pthread_barrierattr_t {
    int pshared;
} pthread_barrierattr_t;

OPAL_WIN32_DECLSPEC int pthread_barrier_init(pthread_barrier_t *b, const pthread_barrierattr_t *attr,
                         unsigned count);
OPAL_WIN32_DECLSPEC int pthread_barrier_destroy(pthread_barrier_t *b);
OPAL_WIN32_DECLSPEC int pthread_barrier_wait(pthread_barrier_t *b);
OPAL_WIN32_DECLSPEC int pthread_barrierattr_init(pthread_barrierattr_t *attr);
OPAL_WIN32_DECLSPEC int pthread_barrierattr_destroy(pthread_barrierattr_t *attr);
OPAL_WIN32_DECLSPEC int pthread_barrierattr_setpshared(pthread_barrierattr_t *attr, int pshared);
OPAL_WIN32_DECLSPEC int pthread_barrierattr_getpshared(const pthread_barrierattr_t *attr, int *pshared);

#define PTHREAD_BARRIER_SERIAL_THREAD (-1)

/* ------------------------------------------------------------------ */
/* Signal masks (stubs -- see signal.h shadow header)                  */
/* ------------------------------------------------------------------ */

OPAL_WIN32_DECLSPEC int pthread_sigmask(int how, const sigset_t *set, sigset_t *oldset);
OPAL_WIN32_DECLSPEC int pthread_sigqueue(pthread_t thread, int sig, const union sigval value);

/* Affinity (stubs for now; hwloc owns affinity on Windows).
 * cpu_set_t and the CPU_* macros live in <sched.h>. */
OPAL_WIN32_DECLSPEC int pthread_setaffinity_np(pthread_t thread, size_t cpusetsize, const cpu_set_t *cpuset);
OPAL_WIN32_DECLSPEC int pthread_getaffinity_np(pthread_t thread, size_t cpusetsize, cpu_set_t *cpuset);

#ifdef __cplusplus
}
#endif

#endif /* OPAL_WIN32_PTHREAD_H */
