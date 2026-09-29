/*
 * Copyright (c) 2026      Amazon.com, Inc. or its affiliates.
 *                         All Rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 *
 * pthreads implementation over Win32 primitives for the native
 * Windows build.  See opal/win32/include/pthread.h for the ABI.
 */

#include "opal_config.h"

#ifdef _WIN32

#    include "opal/win32/opal_win32.h"

/* this TU defines the functions; neutralize shadow macros */
#    undef pthread_create
#    undef pthread_join
#    undef pthread_detach
#    undef pthread_exit
#    undef pthread_self
#    undef pthread_equal
#    undef pthread_kill
#    undef pthread_cancel
#    undef pthread_setcancelstate
#    undef pthread_setcanceltype
#    undef pthread_testcancel
#    undef pthread_atfork
#    undef pthread_setname_np
#    undef pthread_getname_np
#    undef pthread_attr_init
#    undef pthread_attr_destroy
#    undef pthread_attr_setdetachstate
#    undef pthread_attr_getdetachstate
#    undef pthread_attr_setstacksize
#    undef pthread_attr_getstacksize
#    undef pthread_attr_setguardsize
#    undef pthread_attr_getguardsize
#    undef pthread_attr_setscope
#    undef pthread_attr_getscope
#    undef pthread_attr_setschedparam
#    undef pthread_attr_getschedparam
#    undef pthread_attr_setschedpolicy
#    undef pthread_attr_getschedpolicy
#    undef pthread_attr_setinheritsched
#    undef pthread_attr_getinheritsched
#    undef pthread_attr_setstackaddr
#    undef pthread_attr_getstackaddr
#    undef pthread_attr_setstack
#    undef pthread_attr_getstack
#    undef pthread_mutex_init
#    undef pthread_mutex_destroy
#    undef pthread_mutex_timedlock
#    undef pthread_cond_init
#    undef pthread_cond_destroy
#    undef pthread_cond_timedwait
#    undef pthread_condattr_init
#    undef pthread_condattr_destroy
#    undef pthread_condattr_setclock
#    undef pthread_condattr_getclock
#    undef pthread_condattr_setpshared
#    undef pthread_condattr_getpshared
#    undef pthread_once
#    undef pthread_key_create
#    undef pthread_key_delete
#    undef pthread_setspecific
#    undef pthread_getspecific
#    undef pthread_barrier_init
#    undef pthread_barrier_destroy
#    undef pthread_barrier_wait
#    undef pthread_barrierattr_init
#    undef pthread_barrierattr_destroy
#    undef pthread_barrierattr_setpshared
#    undef pthread_barrierattr_getpshared
#    undef pthread_rwlock_init
#    undef pthread_rwlock_destroy
#    undef pthread_rwlock_rdlock
#    undef pthread_rwlock_wrlock
#    undef pthread_rwlock_tryrdlock
#    undef pthread_rwlock_trywrlock
#    undef pthread_rwlock_unlock
#    undef pthread_rwlockattr_init
#    undef pthread_rwlockattr_destroy
#    undef pthread_rwlockattr_setpshared
#    undef pthread_rwlockattr_getpshared
#    undef pthread_sigmask
#    undef pthread_sigqueue
#    undef pthread_getschedparam
#    undef pthread_setschedparam
#    undef pthread_setschedprio
#    undef pthread_getconcurrency
#    undef pthread_setconcurrency
#    undef pthread_setaffinity_np
#    undef pthread_getaffinity_np

/* ================================================================== */
/* thread identity                                                    */
/* ================================================================== */

/* FLS slot holding the pthread_t for the current thread */
static DWORD self_fls_idx = FLS_OUT_OF_INDEXES;
static INIT_ONCE self_fls_once = INIT_ONCE_STATIC_INIT;

static BOOL CALLBACK self_fls_init_cb(PINIT_ONCE once, PVOID param, PVOID *ctx)
{
    (void) once;
    (void) param;
    (void) ctx;
    self_fls_idx = FlsAlloc(NULL);
    return TRUE;
}

static void ensure_self_fls(void)
{
    InitOnceExecuteOnce(&self_fls_once, self_fls_init_cb, NULL, NULL);
}

OPAL_WIN32_DECLSPEC pthread_t pthread_self(void)
{
    struct opal_win32_thread *t;
    ensure_self_fls();
    t = (struct opal_win32_thread *) FlsGetValue(self_fls_idx);
    if (NULL == t) {
        /* foreign thread (e.g., the main thread) -- fabricate a handle */
        t = calloc(1, sizeof(*t));
        if (NULL == t) {
            return NULL;
        }
        t->handle = NULL;
        t->id = GetCurrentThreadId();
        FlsSetValue(self_fls_idx, t);
    }
    return t;
}

OPAL_WIN32_DECLSPEC int pthread_equal(pthread_t t1, pthread_t t2)
{
    if (t1 == t2) {
        return 1;
    }
    if (NULL == t1 || NULL == t2) {
        return 0;
    }
    return t1->id == t2->id;
}

struct pthread_start_ctx {
    struct opal_win32_thread *thr;
    void *(*start)(void *);
    void *arg;
};

static unsigned __stdcall pthread_start_trampoline(void *arg)
{
    struct pthread_start_ctx *ctx = (struct pthread_start_ctx *) arg;
    struct opal_win32_thread *thr = ctx->thr;
    void *(*start)(void *) = ctx->start;
    void *targ = ctx->arg;
    free(ctx);
    ensure_self_fls();
    FlsSetValue(self_fls_idx, thr);
    thr->ret = start(targ);
    InterlockedExchange(&thr->exited, 1);
    /* detached threads own their struct; joined threads transfer it
     * to the joiner */
    if (thr->detached) {
        CloseHandle(thr->handle);
        thr->handle = NULL;
        free(thr);
    }
    return 0;
}

OPAL_WIN32_DECLSPEC int pthread_create(pthread_t *thread, const pthread_attr_t *attr,
                   void *(*start_routine)(void *), void *arg)
{
    struct opal_win32_thread *thr;
    struct pthread_start_ctx *ctx;
    unsigned flags = 0;
    size_t stacksize = 0;
    unsigned tid;
    uintptr_t h;

    thr = calloc(1, sizeof(*thr));
    ctx = calloc(1, sizeof(*ctx));
    if (NULL == thr || NULL == ctx) {
        free(thr);
        free(ctx);
        return ENOMEM;
    }
    ctx->thr = thr;
    ctx->start = start_routine;
    ctx->arg = arg;
    thr->start = start_routine;
    thr->arg = arg;
    if (attr) {
        stacksize = attr->stacksize;
        if (attr->detachstate == PTHREAD_CREATE_DETACHED) {
            thr->detached = 1;
        }
    }
    h = _beginthreadex(NULL, (unsigned) stacksize, pthread_start_trampoline, ctx,
                       flags, &tid);
    if (0 == h) {
        free(thr);
        free(ctx);
        return EAGAIN;
    }
    thr->handle = (HANDLE) h;
    thr->id = (DWORD) tid;
    *thread = thr;
    return 0;
}

OPAL_WIN32_DECLSPEC int pthread_join(pthread_t thread, void **retval)
{
    DWORD rc;
    if (NULL == thread) {
        return EINVAL;
    }
    if (NULL == thread->handle) {
        /* foreign thread that was never pthread_create'd */
        return EINVAL;
    }
    rc = WaitForSingleObject(thread->handle, INFINITE);
    if (WAIT_FAILED == rc) {
        return EINVAL;
    }
    if (retval) {
        *retval = thread->ret;
    }
    CloseHandle(thread->handle);
    free(thread);
    return 0;
}

OPAL_WIN32_DECLSPEC int pthread_detach(pthread_t thread)
{
    if (NULL == thread) {
        return EINVAL;
    }
    InterlockedExchange(&thread->detached, 1);
    return 0;
}

OPAL_WIN32_DECLSPEC void pthread_exit(void *retval)
{
    struct opal_win32_thread *t = pthread_self();
    if (t) {
        t->ret = retval;
        InterlockedExchange(&t->exited, 1);
        if (t->detached && t->handle) {
            CloseHandle(t->handle);
            t->handle = NULL;
            FlsSetValue(self_fls_idx, NULL);
            free(t);
        }
    }
    _endthreadex(0);
    /* not reached */
    ExitThread(0);
}

OPAL_WIN32_DECLSPEC int pthread_kill(pthread_t thread, int sig)
{
    (void) thread;
    /* no signal delivery on Windows; existence check semantics */
    if (0 == sig) {
        return (thread && thread->handle) ? 0 : ESRCH;
    }
    return 0;
}

OPAL_WIN32_DECLSPEC int pthread_cancel(pthread_t thread)
{
    if (NULL == thread) {
        return EINVAL;
    }
    thread->cancel_requested = 1;
    return 0;
}

OPAL_WIN32_DECLSPEC int pthread_setcancelstate(int state, int *oldstate)
{
    (void) state;
    if (oldstate) {
        *oldstate = PTHREAD_CANCEL_ENABLE;
    }
    return 0;
}

OPAL_WIN32_DECLSPEC int pthread_setcanceltype(int type, int *oldtype)
{
    (void) type;
    if (oldtype) {
        *oldtype = PTHREAD_CANCEL_DEFERRED;
    }
    return 0;
}

OPAL_WIN32_DECLSPEC void pthread_testcancel(void)
{
    struct opal_win32_thread *t = pthread_self();
    if (t && t->cancel_requested) {
        pthread_exit(PTHREAD_CANCELED);
    }
}

OPAL_WIN32_DECLSPEC int pthread_atfork(void (*prepare)(void), void (*parent)(void), void (*child)(void))
{
    (void) prepare;
    (void) parent;
    (void) child;
    return 0;
}

OPAL_WIN32_DECLSPEC int pthread_setname_np(pthread_t thread, const char *name)
{
    WCHAR wname[256];
    if (NULL == thread || NULL == thread->handle) {
        return EINVAL;
    }
    MultiByteToWideChar(CP_UTF8, 0, name, -1, wname, 256);
    return SUCCEEDED(SetThreadDescription(thread->handle, wname)) ? 0 : EINVAL;
}

OPAL_WIN32_DECLSPEC int pthread_getname_np(pthread_t thread, char *name, size_t len)
{
    PWSTR wname = NULL;
    if (NULL == thread || NULL == thread->handle || NULL == name || len < 1) {
        return EINVAL;
    }
    if (FAILED(GetThreadDescription(thread->handle, &wname))) {
        name[0] = '\0';
        return EINVAL;
    }
    WideCharToMultiByte(CP_UTF8, 0, wname, -1, name, (int) len, NULL, NULL);
    LocalFree(wname);
    return 0;
}

/* ================================================================== */
/* attributes                                                         */
/* ================================================================== */

OPAL_WIN32_DECLSPEC int pthread_attr_init(pthread_attr_t *attr)
{
    memset(attr, 0, sizeof(*attr));
    attr->detachstate = PTHREAD_CREATE_JOINABLE;
    attr->inheritsched = PTHREAD_INHERIT_SCHED;
    attr->schedpolicy = SCHED_OTHER;
    return 0;
}

OPAL_WIN32_DECLSPEC int pthread_attr_destroy(pthread_attr_t *attr)
{
    (void) attr;
    return 0;
}

OPAL_WIN32_DECLSPEC int pthread_attr_setdetachstate(pthread_attr_t *attr, int detachstate)
{
    attr->detachstate = detachstate;
    return 0;
}

OPAL_WIN32_DECLSPEC int pthread_attr_getdetachstate(const pthread_attr_t *attr, int *detachstate)
{
    *detachstate = attr->detachstate;
    return 0;
}

OPAL_WIN32_DECLSPEC int pthread_attr_setstacksize(pthread_attr_t *attr, size_t stacksize)
{
    attr->stacksize = stacksize;
    return 0;
}

OPAL_WIN32_DECLSPEC int pthread_attr_getstacksize(const pthread_attr_t *attr, size_t *stacksize)
{
    *stacksize = attr->stacksize;
    return 0;
}

OPAL_WIN32_DECLSPEC int pthread_attr_setguardsize(pthread_attr_t *attr, size_t guardsize)
{
    attr->guardsize = (int) guardsize;
    return 0;
}

OPAL_WIN32_DECLSPEC int pthread_attr_getguardsize(const pthread_attr_t *attr, size_t *guardsize)
{
    *guardsize = (size_t) attr->guardsize;
    return 0;
}

OPAL_WIN32_DECLSPEC int pthread_attr_setscope(pthread_attr_t *attr, int scope)
{
    attr->scope = scope;
    return 0;
}

OPAL_WIN32_DECLSPEC int pthread_attr_getscope(const pthread_attr_t *attr, int *scope)
{
    *scope = attr->scope;
    return 0;
}

OPAL_WIN32_DECLSPEC int pthread_attr_setschedparam(pthread_attr_t *attr, const struct sched_param *param)
{
    attr->schedparam = *param;
    return 0;
}

OPAL_WIN32_DECLSPEC int pthread_attr_getschedparam(const pthread_attr_t *attr, struct sched_param *param)
{
    *param = attr->schedparam;
    return 0;
}

OPAL_WIN32_DECLSPEC int pthread_attr_setschedpolicy(pthread_attr_t *attr, int policy)
{
    attr->schedpolicy = policy;
    return 0;
}

OPAL_WIN32_DECLSPEC int pthread_attr_getschedpolicy(const pthread_attr_t *attr, int *policy)
{
    *policy = attr->schedpolicy;
    return 0;
}

OPAL_WIN32_DECLSPEC int pthread_attr_setinheritsched(pthread_attr_t *attr, int inheritsched)
{
    attr->inheritsched = inheritsched;
    return 0;
}

OPAL_WIN32_DECLSPEC int pthread_attr_getinheritsched(const pthread_attr_t *attr, int *inheritsched)
{
    *inheritsched = attr->inheritsched;
    return 0;
}

OPAL_WIN32_DECLSPEC int pthread_attr_setstackaddr(pthread_attr_t *attr, void *stackaddr)
{
    attr->stackaddr = stackaddr;
    return 0;
}

OPAL_WIN32_DECLSPEC int pthread_attr_getstackaddr(const pthread_attr_t *attr, void **stackaddr)
{
    *stackaddr = attr->stackaddr;
    return 0;
}

OPAL_WIN32_DECLSPEC int pthread_attr_setstack(pthread_attr_t *attr, void *stackaddr, size_t stacksize)
{
    attr->stackaddr = stackaddr;
    attr->stacksize = stacksize;
    return 0;
}

OPAL_WIN32_DECLSPEC int pthread_attr_getstack(const pthread_attr_t *attr, void **stackaddr,
                          size_t *stacksize)
{
    *stackaddr = attr->stackaddr;
    *stacksize = attr->stacksize;
    return 0;
}

OPAL_WIN32_DECLSPEC int pthread_getschedparam(pthread_t thread, int *policy, struct sched_param *param)
{
    (void) thread;
    if (policy) {
        *policy = SCHED_OTHER;
    }
    if (param) {
        param->sched_priority = 0;
    }
    return 0;
}

OPAL_WIN32_DECLSPEC int pthread_setschedparam(pthread_t thread, int policy, const struct sched_param *param)
{
    (void) thread;
    (void) policy;
    (void) param;
    return 0;
}

OPAL_WIN32_DECLSPEC int pthread_setschedprio(pthread_t thread, int prio)
{
    (void) prio;
    if (NULL == thread || NULL == thread->handle) {
        return EINVAL;
    }
    return SetThreadPriority(thread->handle, THREAD_PRIORITY_NORMAL) ? 0 : EINVAL;
}

OPAL_WIN32_DECLSPEC int pthread_getconcurrency(void)
{
    return 0;
}

OPAL_WIN32_DECLSPEC int pthread_setconcurrency(int level)
{
    (void) level;
    return 0;
}

/* ================================================================== */
/* mutexes                                                            */
/* ================================================================== */

OPAL_WIN32_DECLSPEC int pthread_mutex_init(pthread_mutex_t *m, const pthread_mutexattr_t *attr)
{
    m->_kind = attr ? attr->type : PTHREAD_MUTEX_NORMAL;
    InitializeCriticalSection(&m->_cs);
    InterlockedExchange(&m->_state, 2);
    return 0;
}

OPAL_WIN32_DECLSPEC int pthread_mutex_destroy(pthread_mutex_t *m)
{
    if (2 == m->_state) {
        DeleteCriticalSection(&m->_cs);
        m->_state = 0;
    }
    return 0;
}

OPAL_WIN32_DECLSPEC int pthread_mutex_timedlock(pthread_mutex_t *m, const struct timespec *abstime)
{
    struct timespec now;
    long long remaining_ns;
    DWORD wait_ms;
    if (2 != m->_state) {
        opal_win32_mutex_lazy_init(m);
    }
    for (;;) {
        if (TryEnterCriticalSection(&m->_cs)) {
            return 0;
        }
        clock_gettime(CLOCK_REALTIME, &now);
        remaining_ns = ((LONGLONG) abstime->tv_sec - now.tv_sec) * 1000000000LL
                       + (abstime->tv_nsec - now.tv_nsec);
        if (remaining_ns <= 0) {
            return ETIMEDOUT;
        }
        wait_ms = (DWORD) (remaining_ns / 1000000LL);
        if (wait_ms > 10) {
            wait_ms = 10;
        }
        if (wait_ms == 0) {
            wait_ms = 1;
        }
        Sleep(wait_ms);
    }
}

OPAL_WIN32_DECLSPEC int pthread_mutex_getprioceiling(const pthread_mutex_t *m, int *ceiling)
{
    (void) m;
    *ceiling = 0;
    return 0;
}

OPAL_WIN32_DECLSPEC int pthread_mutex_setprioceiling(pthread_mutex_t *m, int ceiling, int *old)
{
    (void) m;
    (void) ceiling;
    if (old) {
        *old = 0;
    }
    return 0;
}

OPAL_WIN32_DECLSPEC int pthread_mutex_consistent(pthread_mutex_t *m)
{
    (void) m;
    return 0;
}

OPAL_WIN32_DECLSPEC int pthread_mutexattr_init(pthread_mutexattr_t *attr)
{
    memset(attr, 0, sizeof(*attr));
    attr->type = PTHREAD_MUTEX_NORMAL;
    return 0;
}

OPAL_WIN32_DECLSPEC int pthread_mutexattr_destroy(pthread_mutexattr_t *attr)
{
    (void) attr;
    return 0;
}

OPAL_WIN32_DECLSPEC int pthread_mutexattr_settype(pthread_mutexattr_t *attr, int type)
{
    attr->type = type;
    return 0;
}

OPAL_WIN32_DECLSPEC int pthread_mutexattr_gettype(const pthread_mutexattr_t *attr, int *type)
{
    *type = attr->type;
    return 0;
}

OPAL_WIN32_DECLSPEC int pthread_mutexattr_setpshared(pthread_mutexattr_t *attr, int pshared)
{
    attr->pshared = pshared;
    return 0;
}

OPAL_WIN32_DECLSPEC int pthread_mutexattr_getpshared(const pthread_mutexattr_t *attr, int *pshared)
{
    *pshared = attr->pshared;
    return 0;
}

OPAL_WIN32_DECLSPEC int pthread_mutexattr_setrobust(pthread_mutexattr_t *attr, int robust)
{
    attr->robust = robust;
    return 0;
}

OPAL_WIN32_DECLSPEC int pthread_mutexattr_getrobust(const pthread_mutexattr_t *attr, int *robust)
{
    *robust = attr->robust;
    return 0;
}

OPAL_WIN32_DECLSPEC int pthread_mutexattr_setprotocol(pthread_mutexattr_t *attr, int protocol)
{
    attr->protocol = protocol;
    return 0;
}

OPAL_WIN32_DECLSPEC int pthread_mutexattr_getprotocol(const pthread_mutexattr_t *attr, int *protocol)
{
    *protocol = attr->protocol;
    return 0;
}

OPAL_WIN32_DECLSPEC int pthread_mutexattr_setprioceiling(pthread_mutexattr_t *attr, int prioceiling)
{
    attr->prioceiling = prioceiling;
    return 0;
}

OPAL_WIN32_DECLSPEC int pthread_mutexattr_getprioceiling(const pthread_mutexattr_t *attr, int *prioceiling)
{
    *prioceiling = attr->prioceiling;
    return 0;
}

/* ================================================================== */
/* condition variables                                                */
/* ================================================================== */

OPAL_WIN32_DECLSPEC int pthread_cond_init(pthread_cond_t *cond, const pthread_condattr_t *attr)
{
    (void) attr;
    InitializeConditionVariable(cond);
    return 0;
}

OPAL_WIN32_DECLSPEC int pthread_cond_destroy(pthread_cond_t *cond)
{
    (void) cond;
    return 0;
}

OPAL_WIN32_DECLSPEC int pthread_cond_timedwait(pthread_cond_t *cond, pthread_mutex_t *mutex,
                           const struct timespec *abstime)
{
    struct timespec now;
    long long remaining_ns;
    DWORD wait_ms;
    BOOL rc;

    if (2 != mutex->_state) {
        opal_win32_mutex_lazy_init(mutex);
    }
    clock_gettime(CLOCK_REALTIME, &now);
    remaining_ns = ((LONGLONG) abstime->tv_sec - now.tv_sec) * 1000000000LL
                   + (abstime->tv_nsec - now.tv_nsec);
    if (remaining_ns <= 0) {
        return ETIMEDOUT;
    }
    wait_ms = (DWORD) (remaining_ns / 1000000LL);
    if ((remaining_ns % 1000000LL) != 0) {
        wait_ms += 1;
    }
    rc = SleepConditionVariableCS(cond, &mutex->_cs, wait_ms);
    if (rc) {
        return 0;
    }
    return (GetLastError() == ERROR_TIMEOUT) ? ETIMEDOUT : EINVAL;
}

OPAL_WIN32_DECLSPEC int pthread_condattr_init(pthread_condattr_t *attr)
{
    memset(attr, 0, sizeof(*attr));
    attr->clock_id = CLOCK_REALTIME;
    return 0;
}

OPAL_WIN32_DECLSPEC int pthread_condattr_destroy(pthread_condattr_t *attr)
{
    (void) attr;
    return 0;
}

OPAL_WIN32_DECLSPEC int pthread_condattr_setclock(pthread_condattr_t *attr, int clock_id)
{
    attr->clock_id = clock_id;
    return 0;
}

OPAL_WIN32_DECLSPEC int pthread_condattr_getclock(const pthread_condattr_t *attr, int *clock_id)
{
    *clock_id = attr->clock_id;
    return 0;
}

OPAL_WIN32_DECLSPEC int pthread_condattr_setpshared(pthread_condattr_t *attr, int pshared)
{
    attr->pshared = pshared;
    return 0;
}

OPAL_WIN32_DECLSPEC int pthread_condattr_getpshared(const pthread_condattr_t *attr, int *pshared)
{
    *pshared = attr->pshared;
    return 0;
}

/* ================================================================== */
/* once                                                               */
/* ================================================================== */

static BOOL CALLBACK pthread_once_cb(PINIT_ONCE once, PVOID param, PVOID *ctx)
{
    void (*fn)(void) = (void (*)(void)) param;
    (void) once;
    (void) ctx;
    fn();
    return TRUE;
}

OPAL_WIN32_DECLSPEC int pthread_once(pthread_once_t *once, void (*init_routine)(void))
{
    InitOnceExecuteOnce(once, pthread_once_cb, (PVOID) init_routine, NULL);
    return 0;
}

/* ================================================================== */
/* TLS keys                                                           */
/* ================================================================== */

OPAL_WIN32_DECLSPEC int pthread_key_create(pthread_key_t *key, void (*destructor)(void *))
{
    DWORD idx = FlsAlloc((PFLS_CALLBACK_FUNCTION) destructor);
    if (FLS_OUT_OF_INDEXES == idx) {
        return EAGAIN;
    }
    *key = idx;
    return 0;
}

OPAL_WIN32_DECLSPEC int pthread_key_delete(pthread_key_t key)
{
    return FlsFree(key) ? 0 : EINVAL;
}

OPAL_WIN32_DECLSPEC int pthread_setspecific(pthread_key_t key, const void *value)
{
    return FlsSetValue(key, (PVOID) value) ? 0 : EINVAL;
}

void *pthread_getspecific(pthread_key_t key)
{
    return FlsGetValue(key);
}

/* ================================================================== */
/* barriers                                                           */
/* ================================================================== */

OPAL_WIN32_DECLSPEC int pthread_barrier_init(pthread_barrier_t *b, const pthread_barrierattr_t *attr,
                         unsigned count)
{
    (void) attr;
    InitializeCriticalSection(&b->cs);
    InitializeConditionVariable(&b->cv);
    b->total = count;
    b->count = count;
    b->generation = 0;
    InterlockedExchange(&b->_init, 2);
    return 0;
}

static int barrier_ensure_init(pthread_barrier_t *b)
{
    LONG st = InterlockedCompareExchange(&b->_init, 1, 0);
    if (0 == st) {
        InitializeCriticalSection(&b->cs);
        InitializeConditionVariable(&b->cv);
        b->count = b->total;
        InterlockedExchange(&b->_init, 2);
        return 0;
    }
    while (1 == st) {
        Sleep(0);
        st = b->_init;
    }
    return 0;
}

OPAL_WIN32_DECLSPEC int pthread_barrier_destroy(pthread_barrier_t *b)
{
    DeleteCriticalSection(&b->cs);
    b->_init = 0;
    return 0;
}

OPAL_WIN32_DECLSPEC int pthread_barrier_wait(pthread_barrier_t *b)
{
    unsigned gen;
    int serial = 0;
    if (2 != b->_init) {
        barrier_ensure_init(b);
    }
    EnterCriticalSection(&b->cs);
    gen = b->generation;
    if (--b->count == 0) {
        b->generation++;
        b->count = b->total;
        WakeAllConditionVariable(&b->cv);
        serial = 1;
    } else {
        while (gen == b->generation) {
            SleepConditionVariableCS(&b->cv, &b->cs, INFINITE);
        }
    }
    LeaveCriticalSection(&b->cs);
    return serial ? PTHREAD_BARRIER_SERIAL_THREAD : 0;
}

OPAL_WIN32_DECLSPEC int pthread_barrierattr_init(pthread_barrierattr_t *attr)
{
    memset(attr, 0, sizeof(*attr));
    return 0;
}

OPAL_WIN32_DECLSPEC int pthread_barrierattr_destroy(pthread_barrierattr_t *attr)
{
    (void) attr;
    return 0;
}

OPAL_WIN32_DECLSPEC int pthread_barrierattr_setpshared(pthread_barrierattr_t *attr, int pshared)
{
    attr->pshared = pshared;
    return 0;
}

OPAL_WIN32_DECLSPEC int pthread_barrierattr_getpshared(const pthread_barrierattr_t *attr, int *pshared)
{
    *pshared = attr->pshared;
    return 0;
}

/* ================================================================== */
/* rwlocks                                                            */
/* ================================================================== */

static int rwlock_ensure_init(pthread_rwlock_t *l)
{
    LONG st = InterlockedCompareExchange(&l->_init, 1, 0);
    if (0 == st) {
        InitializeCriticalSection(&l->cs);
        InitializeConditionVariable(&l->cv_readers);
        InitializeConditionVariable(&l->cv_writers);
        l->readers = 0;
        l->writer_active = 0;
        l->writers_waiting = 0;
        InterlockedExchange(&l->_init, 2);
        return 0;
    }
    while (1 == st) {
        Sleep(0);
        st = l->_init;
    }
    return 0;
}

OPAL_WIN32_DECLSPEC int pthread_rwlock_init(pthread_rwlock_t *l, const pthread_rwlockattr_t *a)
{
    (void) a;
    l->_init = 0;
    InitializeCriticalSection(&l->cs);
    InitializeConditionVariable(&l->cv_readers);
    InitializeConditionVariable(&l->cv_writers);
    l->readers = 0;
    l->writer_active = 0;
    l->writers_waiting = 0;
    InterlockedExchange(&l->_init, 2);
    return 0;
}

OPAL_WIN32_DECLSPEC int pthread_rwlock_destroy(pthread_rwlock_t *l)
{
    if (2 == l->_init) {
        DeleteCriticalSection(&l->cs);
        l->_init = 0;
    }
    return 0;
}

OPAL_WIN32_DECLSPEC int pthread_rwlock_rdlock(pthread_rwlock_t *l)
{
    if (2 != l->_init) {
        rwlock_ensure_init(l);
    }
    EnterCriticalSection(&l->cs);
    while (l->writer_active || l->writers_waiting > 0) {
        SleepConditionVariableCS(&l->cv_readers, &l->cs, INFINITE);
    }
    l->readers++;
    LeaveCriticalSection(&l->cs);
    return 0;
}

OPAL_WIN32_DECLSPEC int pthread_rwlock_tryrdlock(pthread_rwlock_t *l)
{
    int ok = 0;
    if (2 != l->_init) {
        rwlock_ensure_init(l);
    }
    EnterCriticalSection(&l->cs);
    if (!l->writer_active && 0 == l->writers_waiting) {
        l->readers++;
        ok = 1;
    }
    LeaveCriticalSection(&l->cs);
    return ok ? 0 : EBUSY;
}

OPAL_WIN32_DECLSPEC int pthread_rwlock_wrlock(pthread_rwlock_t *l)
{
    if (2 != l->_init) {
        rwlock_ensure_init(l);
    }
    EnterCriticalSection(&l->cs);
    l->writers_waiting++;
    while (l->writer_active || l->readers > 0) {
        SleepConditionVariableCS(&l->cv_writers, &l->cs, INFINITE);
    }
    l->writers_waiting--;
    l->writer_active = 1;
    LeaveCriticalSection(&l->cs);
    return 0;
}

OPAL_WIN32_DECLSPEC int pthread_rwlock_trywrlock(pthread_rwlock_t *l)
{
    int ok = 0;
    if (2 != l->_init) {
        rwlock_ensure_init(l);
    }
    EnterCriticalSection(&l->cs);
    if (!l->writer_active && 0 == l->readers) {
        l->writer_active = 1;
        ok = 1;
    }
    LeaveCriticalSection(&l->cs);
    return ok ? 0 : EBUSY;
}

OPAL_WIN32_DECLSPEC int pthread_rwlock_unlock(pthread_rwlock_t *l)
{
    EnterCriticalSection(&l->cs);
    if (l->writer_active) {
        l->writer_active = 0;
        if (l->writers_waiting > 0) {
            WakeConditionVariable(&l->cv_writers);
        } else {
            WakeAllConditionVariable(&l->cv_readers);
        }
    } else if (l->readers > 0) {
        if (0 == --l->readers && l->writers_waiting > 0) {
            WakeConditionVariable(&l->cv_writers);
        }
    }
    LeaveCriticalSection(&l->cs);
    return 0;
}

OPAL_WIN32_DECLSPEC int pthread_rwlockattr_init(pthread_rwlockattr_t *attr)
{
    memset(attr, 0, sizeof(*attr));
    return 0;
}

OPAL_WIN32_DECLSPEC int pthread_rwlockattr_destroy(pthread_rwlockattr_t *attr)
{
    (void) attr;
    return 0;
}

OPAL_WIN32_DECLSPEC int pthread_rwlockattr_setpshared(pthread_rwlockattr_t *attr, int pshared)
{
    attr->pshared = pshared;
    return 0;
}

OPAL_WIN32_DECLSPEC int pthread_rwlockattr_getpshared(const pthread_rwlockattr_t *attr, int *pshared)
{
    *pshared = attr->pshared;
    return 0;
}

/* ================================================================== */
/* signals / affinity stubs                                           */
/* ================================================================== */

OPAL_WIN32_DECLSPEC int pthread_sigmask(int how, const sigset_t *set, sigset_t *oldset)
{
    (void) how;
    (void) set;
    if (oldset) {
        *oldset = 0;
    }
    return 0;
}

OPAL_WIN32_DECLSPEC int pthread_sigqueue(pthread_t thread, int sig, const union sigval value)
{
    (void) thread;
    (void) sig;
    (void) value;
    return 0;
}

OPAL_WIN32_DECLSPEC int pthread_setaffinity_np(pthread_t thread, size_t cpusetsize, const cpu_set_t *cpuset)
{
    DWORD_PTR mask = 0;
    int i;
    (void) cpusetsize;
    if (NULL == thread || NULL == thread->handle) {
        return EINVAL;
    }
    for (i = 0; i < 64; i++) {
        if (CPU_ISSET(i, cpuset)) {
            mask |= ((DWORD_PTR) 1) << i;
        }
    }
    return SetThreadAffinityMask(thread->handle, mask) ? 0 : EINVAL;
}

OPAL_WIN32_DECLSPEC int pthread_getaffinity_np(pthread_t thread, size_t cpusetsize, cpu_set_t *cpuset)
{
    (void) thread;
    if (cpusetsize < sizeof(cpu_set_t)) {
        return EINVAL;
    }
    CPU_ZERO(cpuset);
    return 0;
}

OPAL_WIN32_DECLSPEC int opal_win32_cpu_count(const cpu_set_t *s)
{
    int n = 0, i;
    for (i = 0; i < CPU_SETSIZE && i < 512; i++) {
        if (CPU_ISSET(i, s)) {
            n++;
        }
    }
    return n;
}

/* ================================================================== */
/* sched                                                              */
/* ================================================================== */

OPAL_WIN32_DECLSPEC int opal_win32_sched_get_priority_max(int policy)
{
    (void) policy;
    return THREAD_PRIORITY_TIME_CRITICAL;
}

OPAL_WIN32_DECLSPEC int opal_win32_sched_get_priority_min(int policy)
{
    (void) policy;
    return THREAD_PRIORITY_IDLE;
}

OPAL_WIN32_DECLSPEC int opal_win32_sched_setscheduler(pid_t pid, int policy, const struct sched_param *p)
{
    (void) pid;
    (void) policy;
    (void) p;
    return 0;
}

OPAL_WIN32_DECLSPEC int opal_win32_sched_getscheduler(pid_t pid)
{
    (void) pid;
    return SCHED_OTHER;
}

OPAL_WIN32_DECLSPEC int opal_win32_sched_getparam(pid_t pid, struct sched_param *p)
{
    (void) pid;
    memset(p, 0, sizeof(*p));
    return 0;
}

OPAL_WIN32_DECLSPEC int opal_win32_sched_setparam(pid_t pid, const struct sched_param *p)
{
    (void) pid;
    (void) p;
    return 0;
}

OPAL_WIN32_DECLSPEC int opal_win32_sched_setaffinity(pid_t pid, size_t size, const cpu_set_t *set)
{
    DWORD_PTR mask = 0;
    int i;
    (void) pid;
    for (i = 0; i < 64; i++) {
        if (CPU_ISSET(i, set)) {
            mask |= ((DWORD_PTR) 1) << i;
        }
    }
    return SetProcessAffinityMask(GetCurrentProcess(), mask) ? 0 : EINVAL;
}

OPAL_WIN32_DECLSPEC int opal_win32_sched_getaffinity(pid_t pid, size_t size, cpu_set_t *set)
{
    DWORD_PTR pm, sm;
    int i;
    (void) pid;
    (void) size;
    CPU_ZERO(set);
    if (GetProcessAffinityMask(GetCurrentProcess(), &pm, &sm)) {
        for (i = 0; i < 64; i++) {
            if (pm & (((DWORD_PTR) 1) << i)) {
                CPU_SET(i, set);
            }
        }
    }
    return 0;
}

OPAL_WIN32_DECLSPEC int opal_win32_sched_rr_get_interval(pid_t pid, struct timespec *t)
{
    (void) pid;
    t->tv_sec = 0;
    t->tv_nsec = 20000000; /* ~20ms default quantum */
    return 0;
}

#endif /* _WIN32 */
