/*
 * Copyright (c) 2026      Amazon.com, Inc. or its affiliates.
 *                         All Rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 *
 * Process management: exec/spawn via CreateProcess, waitpid registry,
 * kill(), and the sigaction shim.  Also serves as the home for
 * posix_spawn(), which the higher layers use to start processes.
 */

#include "opal_config.h"

#ifdef _WIN32

#    include "opal/win32/opal_win32.h"

#    undef execv
#    undef execve
#    undef execvp
#    undef execl
#    undef execle
#    undef execlp
#    undef execlpe
#    undef fork
#    undef vfork
#    undef waitpid
#    undef wait
#    undef waitid
#    undef wait3
#    undef wait4
#    undef kill
#    undef sigaction
#    undef sigemptyset
#    undef sigfillset
#    undef sigaddset
#    undef sigdelset
#    undef sigismember
#    undef sigprocmask
#    undef sigpending
#    undef sigsuspend
#    undef posix_spawn
#    undef posix_spawnp

/* ================================================================== */
/* child process registry for waitpid()                               */
/* ================================================================== */

struct child_ent {
    pid_t  pid;
    HANDLE handle;
    int    reaped;      /* waitpid already consumed the exit status */
    int    status;
    struct child_ent *next;
};

static CRITICAL_SECTION child_lock;
static volatile LONG child_lock_init = 0;
static struct child_ent *child_list = NULL;

static void child_lock_once(void)
{
    if (0 == InterlockedCompareExchange(&child_lock_init, 1, 0)) {
        InitializeCriticalSection(&child_lock);
        InterlockedExchange(&child_lock_init, 2);
    }
    while (1 == child_lock_init) {
        Sleep(0);
    }
}

OPAL_WIN32_DECLSPEC void opal_win32_register_child(pid_t pid, HANDLE handle)
{
    struct child_ent *e;
    child_lock_once();
    e = calloc(1, sizeof(*e));
    if (NULL == e) {
        return;
    }
    e->pid = pid;
    e->handle = handle;
    EnterCriticalSection(&child_lock);
    e->next = child_list;
    child_list = e;
    LeaveCriticalSection(&child_lock);
}

OPAL_WIN32_DECLSPEC void opal_win32_unregister_child(pid_t pid)
{
    struct child_ent **pp, *e;
    child_lock_once();
    EnterCriticalSection(&child_lock);
    for (pp = &child_list; NULL != (e = *pp); pp = &e->next) {
        if (e->pid == pid) {
            *pp = e->next;
            if (e->handle) {
                CloseHandle(e->handle);
            }
            free(e);
            break;
        }
    }
    LeaveCriticalSection(&child_lock);
}

static int child_exit_status(DWORD code)
{
    /* encode like wait(): exit code in high byte */
    return (int) ((code & 0xff) << 8);
}

OPAL_WIN32_DECLSPEC pid_t opal_win32_waitpid(pid_t pid, int *status, int options)
{
    struct child_ent *e, *hit;
    int seen;

    child_lock_once();
    for (;;) {
        hit = NULL;
        seen = 0;
        EnterCriticalSection(&child_lock);
        for (e = child_list; e; e = e->next) {
            if (e->reaped) {
                continue;
            }
            if (pid > 0 && e->pid != pid) {
                continue;
            }
            /* a candidate: either the requested pid or, for pid <= 0,
             * each registered child in turn until one proves exited */
            seen = 1;
            if (WAIT_OBJECT_0 == WaitForSingleObject(e->handle, 0)) {
                /* mark reaped inside the lock so two threads cannot
                 * both claim the same exit status */
                hit = e;
                e->reaped = 1;
            }
            if (pid > 0 || NULL != hit) {
                break;
            }
        }
        LeaveCriticalSection(&child_lock);
        if (NULL != hit) {
            DWORD code = 0;
            /* the winning probe marked the entry while holding the lock;
             * GetExitCodeProcess on a signaled process handle always
             * succeeds */
            GetExitCodeProcess(hit->handle, &code);
            hit->status = child_exit_status(code);
            if (status) {
                *status = hit->status;
            }
            return hit->pid;
        }
        if (!seen) {
            /* unknown pid (or registry empty) */
            errno = ECHILD;
            return -1;
        }
        if (options & WNOHANG) {
            return 0;
        }
        Sleep(10);
    }
}

OPAL_WIN32_DECLSPEC pid_t opal_win32_wait(int *status)
{
    return opal_win32_waitpid(-1, status, 0);
}

OPAL_WIN32_DECLSPEC int opal_win32_waitid(int idtype, int id, void *infop, int options)
{
    (void) idtype;
    (void) infop;
    return opal_win32_waitpid(id, NULL, options);
}

OPAL_WIN32_DECLSPEC pid_t opal_win32_wait3(int *status, int options, void *rusage)
{
    (void) rusage;
    return opal_win32_waitpid(-1, status, options);
}

OPAL_WIN32_DECLSPEC pid_t opal_win32_wait4(pid_t pid, int *status, int options, void *rusage)
{
    (void) rusage;
    return opal_win32_waitpid(pid, status, options);
}

/* ================================================================== */
/* exec and spawn via CreateProcess                                    */
/* ================================================================== */

/* build a single Windows command line from argv, quoting args that
 * contain spaces/empty strings */
static int build_cmdline(char *const argv[], char *out, size_t outlen)
{
    size_t pos = 0;
    int i;
    for (i = 0; argv[i]; i++) {
        const char *a = argv[i];
        int need_quote = (NULL != strchr(a, ' ')) || (NULL != strchr(a, '\t'))
                         || ('\0' == *a);
        if (i > 0) {
            if (pos + 1 >= outlen) {
                return -1;
            }
            out[pos++] = ' ';
        }
        if (need_quote && pos + 1 < outlen) {
            out[pos++] = '"';
        }
        while (*a) {
            char c = *a;
            if ('"' == c || ('\\' == c && (a[1] == '"' || a[1] == '\0'))) {
                if (pos + 1 >= outlen) {
                    return -1;
                }
                out[pos++] = '\\';
            }
            if (pos + 1 >= outlen) {
                return -1;
            }
            out[pos++] = c;
            a++;
        }
        if (need_quote && pos + 1 < outlen) {
            out[pos++] = '"';
        }
    }
    out[pos] = '\0';
    return 0;
}

/* Spawn a process and register it for waitpid().  Returns the pid or
 * -1.  env==NULL inherits our environment.  When suspended is nonzero
 * the process is created with CREATE_SUSPENDED and the initial thread's
 * handle is handed out through *outthread (caller's to CloseHandle after
 * ResumeThread) so the caller can publish the pid before the child can
 * possibly exit.  outthread is also honored when suspended==0, in which
 * case the caller owns (and must close) a live thread handle. */
OPAL_WIN32_DECLSPEC pid_t opal_win32_spawn_process(const char *path, char *const argv[],
                               char *const envp[], const char *cwd,
                               HANDLE hStdin, HANDLE hStdout, HANDLE hStderr,
                               int suspended, HANDLE *outhandle, HANDLE *outthread)
{
    char cmdline[32768];
    STARTUPINFOEXA si;
    PROCESS_INFORMATION pi;
    char *envblock = NULL;
    BOOL inherit;
    DWORD cflags = 0;
    pid_t pid;

    if (0 != build_cmdline(argv, cmdline, sizeof(cmdline))) {
        errno = E2BIG;
        return -1;
    }
    memset(&si, 0, sizeof(si));
    inherit = (hStdin != NULL || hStdout != NULL || hStderr != NULL);
    if (inherit) {
        HANDLE hlist[3];
        SIZE_T attrsize = 0;
        int nh = 0;

        si.StartupInfo.dwFlags |= STARTF_USESTDHANDLES;
        si.StartupInfo.hStdInput = hStdin ? hStdin : GetStdHandle(STD_INPUT_HANDLE);
        si.StartupInfo.hStdOutput = hStdout ? hStdout : GetStdHandle(STD_OUTPUT_HANDLE);
        si.StartupInfo.hStdError = hStderr ? hStderr : GetStdHandle(STD_ERROR_HANDLE);
        hlist[0] = si.StartupInfo.hStdInput;
        hlist[1] = si.StartupInfo.hStdOutput;
        hlist[2] = si.StartupInfo.hStdError;
        nh = 3;

        /* The listed handles must be inheritable for the child to
         * receive them; our callers hand us CRT pipe ends created
         * without HANDLE_FLAG_INHERIT. */
        for (int i = 0; i < nh; i++) {
            SetHandleInformation(hlist[i], HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT);
        }

        /* bInheritHandles=TRUE would pass EVERY inheritable handle in
         * this process (sockets, other children's pipe ends - a leaked
         * pipe write end means the parent's reader never sees EOF).
         * PROC_THREAD_ATTRIBUTE_HANDLE_LIST restricts inheritance to
         * exactly the std handles we named. */
        InitializeProcThreadAttributeList(NULL, 1, 0, &attrsize);
        si.lpAttributeList = (PPROC_THREAD_ATTRIBUTE_LIST) malloc(attrsize);
        if (NULL != si.lpAttributeList
            && InitializeProcThreadAttributeList(si.lpAttributeList, 1, 0, &attrsize)
            && UpdateProcThreadAttribute(si.lpAttributeList, 0,
                                         PROC_THREAD_ATTRIBUTE_HANDLE_LIST, hlist,
                                         nh * sizeof(HANDLE), NULL, NULL)) {
            si.StartupInfo.cb = sizeof(si);
            cflags |= EXTENDED_STARTUPINFO_PRESENT;
        } else {
            /* attribute lists unavailable - fall back to plain
             * inheritance and accept the leak */
            free(si.lpAttributeList);
            si.lpAttributeList = NULL;
            si.StartupInfo.cb = sizeof(si.StartupInfo);
        }
    } else {
        si.StartupInfo.cb = sizeof(si.StartupInfo);
    }
    if (suspended) {
        cflags |= CREATE_SUSPENDED;
    }
    /* envp -> "K=V\0K=V\0\0" block */
    if (envp) {
        size_t total = 1, pos = 0;
        int i;
        for (i = 0; envp[i]; i++) {
            total += strlen(envp[i]) + 1;
        }
        envblock = malloc(total);
        if (NULL != envblock) {
            for (i = 0; envp[i]; i++) {
                size_t l = strlen(envp[i]) + 1;
                memcpy(envblock + pos, envp[i], l);
                pos += l;
            }
            envblock[pos] = '\0';
        }
    }
    if (!CreateProcessA(path, cmdline, NULL, NULL, inherit, cflags,
                        envblock, cwd, &si.StartupInfo, &pi)) {
        DWORD gle = GetLastError();
        switch (gle) {
        case ERROR_FILE_NOT_FOUND:
        case ERROR_PATH_NOT_FOUND:
            errno = ENOENT;
            break;
        case ERROR_ACCESS_DENIED:
            errno = EACCES;
            break;
        case ERROR_BAD_EXE_FORMAT:
            errno = ENOEXEC;
            break;
        case ERROR_NOT_ENOUGH_MEMORY:
        case ERROR_OUTOFMEMORY:
            errno = ENOMEM;
            break;
        default:
            errno = EINVAL;
            break;
        }
        free(envblock);
        if (NULL != si.lpAttributeList) {
            DeleteProcThreadAttributeList(si.lpAttributeList);
            free(si.lpAttributeList);
        }
        return -1;
    }
    free(envblock);
    if (NULL != si.lpAttributeList) {
        DeleteProcThreadAttributeList(si.lpAttributeList);
        free(si.lpAttributeList);
    }
    pid = (pid_t) pi.dwProcessId;
    if (NULL != outthread) {
        *outthread = pi.hThread;
    } else {
        CloseHandle(pi.hThread);
    }
    /* the registry owns pi.hProcess - it is what waitpid() waits on.
     * A caller wanting the handle (e.g. to SetProcessAffinityMask on a
     * suspended child) gets a duplicate so closing it cannot invalidate
     * the reaper's copy. */
    opal_win32_register_child(pid, pi.hProcess);
    if (outhandle) {
        if (!DuplicateHandle(GetCurrentProcess(), pi.hProcess,
                             GetCurrentProcess(), outhandle, 0, FALSE,
                             DUPLICATE_SAME_ACCESS)) {
            *outhandle = NULL;
        }
    }
    return pid;
}

OPAL_WIN32_DECLSPEC int opal_win32_execv(const char *path, char *const argv[])
{
    return opal_win32_execve(path, argv, NULL);
}

OPAL_WIN32_DECLSPEC int opal_win32_execvp(const char *file, char *const argv[])
{
    return opal_win32_execve(file, argv, NULL);
}

OPAL_WIN32_DECLSPEC int opal_win32_execve(const char *path, char *const argv[], char *const envp[])
{
    /* POSIX exec* never returns on success.  On Windows we spawn and
     * then wait, exiting with the child's code -- this is only ever
     * used for "exec a helper" paths (launcher internals). */
    pid_t pid = opal_win32_spawn_process(path, argv, envp, NULL, NULL, NULL, NULL,
                                         0, NULL, NULL);
    int status;
    if (pid < 0) {
        return -1;
    }
    opal_win32_waitpid(pid, &status, 0);
    ExitProcess(WIFEXITED(status) ? WEXITSTATUS(status) : 1);
    /* not reached */
    return -1;
}

OPAL_WIN32_DECLSPEC int opal_win32_execl_stub(const char *path, const char *arg0, ...)
{
    (void) path;
    (void) arg0;
    errno = ENOSYS;
    return -1;
}

OPAL_WIN32_DECLSPEC int opal_win32_execle_stub(const char *path, const char *arg0, ...)
{
    (void) path;
    (void) arg0;
    errno = ENOSYS;
    return -1;
}

OPAL_WIN32_DECLSPEC int opal_win32_execlp_stub(const char *file, const char *arg0, ...)
{
    (void) file;
    (void) arg0;
    errno = ENOSYS;
    return -1;
}

OPAL_WIN32_DECLSPEC int opal_win32_execlpe_stub(const char *file, const char *arg0, ...)
{
    (void) file;
    (void) arg0;
    errno = ENOSYS;
    return -1;
}

OPAL_WIN32_DECLSPEC pid_t opal_win32_fork_stub(void)
{
    errno = ENOSYS;
    return -1;
}

/* ================================================================== */
/* posix_spawn                                                        */
/* ================================================================== */

OPAL_WIN32_DECLSPEC int posix_spawn(pid_t *pid, const char *path,
                const posix_spawn_file_actions_t *file_actions,
                const posix_spawnattr_t *attrp, char *const argv[],
                char *const envp[])
{
    (void) file_actions;
    (void) attrp;
    pid_t p = opal_win32_spawn_process(path, argv, envp, NULL, NULL, NULL, NULL,
                                       0, NULL, NULL);
    if (p < 0) {
        return errno;
    }
    if (pid) {
        *pid = p;
    }
    return 0;
}

OPAL_WIN32_DECLSPEC int posix_spawnp(pid_t *pid, const char *file,
                 const posix_spawn_file_actions_t *file_actions,
                 const posix_spawnattr_t *attrp, char *const argv[],
                 char *const envp[])
{
    return posix_spawn(pid, file, file_actions, attrp, argv, envp);
}

/* ================================================================== */
/* kill() / signal glue                                               */
/* ================================================================== */

OPAL_WIN32_DECLSPEC int opal_win32_kill(pid_t pid, int sig)
{
    HANDLE h;
    if (0 == pid) {
        /* broadcast signals unsupported */
        errno = EINVAL;
        return -1;
    }
    if (0 > pid) {
        /* a negative pid targets a process group on POSIX.  Windows
         * has no process groups; the best available approximation is
         * signalling the group leader alone */
        pid = -pid;
    }
    switch (sig) {
    case 0:
        /* existence check */
        h = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, (DWORD) pid);
        if (NULL == h) {
            errno = ESRCH;
            return -1;
        }
        CloseHandle(h);
        return 0;
    case SIGKILL:
    case SIGTERM:
    case SIGINT:
        h = OpenProcess(PROCESS_TERMINATE, FALSE, (DWORD) pid);
        if (NULL == h) {
            errno = ESRCH;
            return -1;
        }
        TerminateProcess(h, (UINT) (128 + sig));
        CloseHandle(h);
        return 0;
    default:
        /* all other signals are silently ignored -- the semantics we
         * can actually deliver are covered by the cases above */
        return 0;
    }
}

/* sigaction: only a few signals are deliverable on Windows; those map
 * onto CRT signal().  The rest are recorded and ignored. */
struct sigaction_entry {
    int installed;
    void (*handler)(int);
};
static struct sigaction_entry sigaction_table[64];

OPAL_WIN32_DECLSPEC int sigaction(int sig, const struct sigaction *act, struct sigaction *oact)
{
    if (sig < 0 || sig >= 64) {
        errno = EINVAL;
        return -1;
    }
    if (oact) {
        memset(oact, 0, sizeof(*oact));
        if (sigaction_table[sig].installed) {
            oact->sa_handler = sigaction_table[sig].handler;
        } else {
            oact->sa_handler = SIG_DFL;
        }
    }
    if (act) {
        sigaction_table[sig].installed = 1;
        sigaction_table[sig].handler = act->sa_handler;
        switch (sig) {
        case SIGINT:
        case SIGTERM:
        case SIGABRT:
        case SIGFPE:
        case SIGILL:
        case SIGSEGV:
        case SIGBREAK:
            signal(sig, act->sa_handler);
            break;
        default:
            break;
        }
    }
    return 0;
}

OPAL_WIN32_DECLSPEC int sigemptyset(sigset_t *set)
{
    *set = 0;
    return 0;
}

OPAL_WIN32_DECLSPEC int sigfillset(sigset_t *set)
{
    *set = ~0UL;
    return 0;
}

OPAL_WIN32_DECLSPEC int sigaddset(sigset_t *set, int signo)
{
    if (signo < 0 || signo >= (int) (8 * sizeof(sigset_t))) {
        errno = EINVAL;
        return -1;
    }
    *set |= (1UL << signo);
    return 0;
}

OPAL_WIN32_DECLSPEC int sigdelset(sigset_t *set, int signo)
{
    if (signo < 0 || signo >= (int) (8 * sizeof(sigset_t))) {
        errno = EINVAL;
        return -1;
    }
    *set &= ~(1UL << signo);
    return 0;
}

OPAL_WIN32_DECLSPEC int sigismember(const sigset_t *set, int signo)
{
    if (signo < 0 || signo >= (int) (8 * sizeof(sigset_t))) {
        return 0;
    }
    return (*set & (1UL << signo)) ? 1 : 0;
}

OPAL_WIN32_DECLSPEC int sigprocmask(int how, const sigset_t *set, sigset_t *oldset)
{
    static sigset_t cur = 0;
    (void) how;
    if (oldset) {
        *oldset = cur;
    }
    if (set) {
        cur = *set;
    }
    return 0;
}

OPAL_WIN32_DECLSPEC int sigpending(sigset_t *set)
{
    *set = 0;
    return 0;
}

OPAL_WIN32_DECLSPEC int sigsuspend(const sigset_t *mask)
{
    (void) mask;
    Sleep(INFINITE);
    return -1;
}

OPAL_WIN32_DECLSPEC int sigwait(const sigset_t *set, int *sig)
{
    (void) set;
    (void) sig;
    return -1;
}

OPAL_WIN32_DECLSPEC int sigqueue(pid_t pid, int sig, const union sigval value)
{
    (void) value;
    return opal_win32_kill(pid, sig);
}

OPAL_WIN32_DECLSPEC int siginterrupt(int sig, int flag)
{
    (void) sig;
    (void) flag;
    return 0;
}

OPAL_WIN32_DECLSPEC int sighold(int sig)
{
    (void) sig;
    return 0;
}

OPAL_WIN32_DECLSPEC int sigrelse(int sig)
{
    (void) sig;
    return 0;
}

OPAL_WIN32_DECLSPEC int sigignore(int sig)
{
    (void) sig;
    return 0;
}

OPAL_WIN32_DECLSPEC int sigpause(int sig, int flag)
{
    (void) sig;
    (void) flag;
    return 0;
}

char *strsignal(int sig)
{
    static char buf[32];
    snprintf(buf, sizeof(buf), "Signal %d", sig);
    return buf;
}

OPAL_WIN32_DECLSPEC void psignal(int sig, const char *msg)
{
    fprintf(stderr, "%s: Signal %d\n", msg ? msg : "", sig);
}

#endif /* _WIN32 */
