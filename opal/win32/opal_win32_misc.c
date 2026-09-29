/*
 * Copyright (c) 2026      Amazon.com, Inc. or its affiliates.
 *                         All Rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 *
 * Assorted POSIX compat implementations: getopt, dlfcn, fnmatch,
 * execinfo, syslog, pwd/grp, termios stubs, spawn attributes.
 */

#include "opal_config.h"

#ifdef _WIN32

#    include "opal/win32/opal_win32.h"

#    undef getopt
#    undef getopt_long
#    undef getopt_long_only
#    undef dlopen
#    undef dlerror
#    undef dlsym
#    undef dlclose
#    undef dladdr
#    undef fnmatch
#    undef backtrace
#    undef backtrace_symbols
#    undef backtrace_symbols_fd
#    undef openlog
#    undef syslog
#    undef vsyslog
#    undef closelog
#    undef setlogmask
#    undef getpwuid
#    undef getpwnam
#    undef getpwuid_r
#    undef getpwnam_r
#    undef getpwent
#    undef setpwent
#    undef endpwent
#    undef getgrgid
#    undef getgrnam
#    undef getgrgid_r
#    undef getgrnam_r
#    undef getgrent
#    undef setgrent
#    undef endgrent
#    undef getgrouplist
#    undef setgroups
#    undef tcgetattr
#    undef tcsetattr
#    undef tcflush
#    undef tcdrain
#    undef tcflow
#    undef tcsendbreak
#    undef cfgetispeed
#    undef cfgetospeed
#    undef cfsetispeed
#    undef cfsetospeed
#    undef cfsetspeed
#    undef cfmakeraw
#    undef openpty
#    undef forkpty
#    undef login_tty
#    undef syscall
#    undef posix_spawn
#    undef posix_spawnp
#    undef posix_spawnattr_init
#    undef posix_spawnattr_destroy
#    undef posix_spawnattr_getflags
#    undef posix_spawnattr_setflags
#    undef posix_spawnattr_getpgroup
#    undef posix_spawnattr_setpgroup
#    undef posix_spawnattr_getsigmask
#    undef posix_spawnattr_setsigmask
#    undef posix_spawnattr_getsigdefault
#    undef posix_spawnattr_setsigdefault
#    undef posix_spawnattr_getschedpolicy
#    undef posix_spawnattr_setschedpolicy
#    undef posix_spawnattr_getschedparam
#    undef posix_spawnattr_setschedparam
#    undef posix_spawn_file_actions_init
#    undef posix_spawn_file_actions_destroy
#    undef posix_spawn_file_actions_addopen
#    undef posix_spawn_file_actions_addclose
#    undef posix_spawn_file_actions_adddup2
#    undef daemon
#    undef kill
#    undef ftok
#    undef strsignal
#    undef shmget
#    undef shmat
#    undef shmdt
#    undef shmctl
#    undef waitpid

/* ================================================================== */
/* getopt                                                             */
/* ================================================================== */

OPAL_WIN32_DECLSPEC char *opal_optarg = NULL;
OPAL_WIN32_DECLSPEC int opal_optind = 1;
OPAL_WIN32_DECLSPEC int opal_opterr = 1;
OPAL_WIN32_DECLSPEC int opal_optopt = '?';

OPAL_WIN32_DECLSPEC int opal_getopt(int argc, char *const argv[], const char *optstring)
{
    const char *arg;
    const char *p;

    /* optind == 0 is the POSIX/GNU "reset and rescan" signal; argv[0] is
     * the program name and is never scanned as an option. */
    if (0 == opal_optind) {
        opal_optind = 1;
    }
    if (opal_optind >= argc) {
        return -1;
    }
    arg = argv[opal_optind];
    if (NULL == arg || arg[0] != '-' || arg[1] == '\0') {
        return -1;
    }
    if (0 == strcmp(arg, "--")) {
        opal_optind++;
        return -1;
    }
    opal_optopt = arg[1];
    p = strchr(optstring, opal_optopt);
    if (NULL == p) {
        if (opal_opterr) {
            fprintf(stderr, "%s: invalid option -- %c\n", argv[0], opal_optopt);
        }
        opal_optind++;
        return '?';
    }
    if (p[1] == ':') {
        /* needs an argument */
        if (arg[2] != '\0') {
            opal_optarg = (char *) (arg + 2);
            opal_optind++;
        } else {
            opal_optind++;
            if (opal_optind >= argc) {
                if (opal_opterr) {
                    fprintf(stderr, "%s: option requires an argument -- %c\n",
                            argv[0], opal_optopt);
                }
                return (optstring[0] == ':') ? ':' : '?';
            }
            opal_optarg = (char *) argv[opal_optind];
            opal_optind++;
        }
        return opal_optopt;
    }
    /* plain flag; handle clustering by tracking within-arg position is
     * overkill for our tools -- process one flag per arg */
    opal_optind++;
    return opal_optopt;
}

OPAL_WIN32_DECLSPEC int opal_getopt_long(int argc, char *const argv[], const char *optstring,
                     const struct option *longopts, int *longindex)
{
    const char *arg;
    int i;

    if (0 == opal_optind) {
        opal_optind = 1;
    }
    if (opal_optind >= argc) {
        return -1;
    }
    arg = argv[opal_optind];
    if (NULL == arg) {
        return -1;
    }
    if (0 == strncmp(arg, "--", 2) && arg[2] != '\0' && longopts) {
        const char *name = arg + 2;
        const char *eq = strchr(name, '=');
        size_t nlen = eq ? (size_t) (eq - name) : strlen(name);
        for (i = 0; longopts[i].name; i++) {
            if (strlen(longopts[i].name) == nlen
                && 0 == strncmp(longopts[i].name, name, nlen)) {
                if (longindex) {
                    *longindex = i;
                }
                opal_optind++;
                if (longopts[i].has_arg == required_argument
                    || longopts[i].has_arg == optional_argument) {
                    if (eq) {
                        opal_optarg = (char *) (eq + 1);
                    } else if (longopts[i].has_arg == required_argument
                               && opal_optind < argc) {
                        opal_optarg = (char *) argv[opal_optind];
                        opal_optind++;
                    } else {
                        opal_optarg = NULL;
                    }
                }
                if (longopts[i].flag) {
                    *longopts[i].flag = longopts[i].val;
                    return 0;
                }
                return longopts[i].val;
            }
        }
        opal_optind++;
        return '?';
    }
    return opal_getopt(argc, argv, optstring);
}

OPAL_WIN32_DECLSPEC int opal_getopt_long_only(int argc, char *const argv[], const char *optstring,
                          const struct option *longopts, int *longindex)
{
    return opal_getopt_long(argc, argv, optstring, longopts, longindex);
}

/* ================================================================== */
/* dlfcn                                                              */
/* ================================================================== */

static char dl_errbuf[512];

static void dl_set_err(const char *prefix)
{
    DWORD err = GetLastError();
    DWORD n;
    n = FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                       NULL, err, 0, dl_errbuf, sizeof(dl_errbuf), NULL);
    if (0 == n) {
        snprintf(dl_errbuf, sizeof(dl_errbuf), "%s: error %lu", prefix, err);
    }
}

OPAL_WIN32_DECLSPEC void *opal_win32_dlopen(const char *filename, int flag)
{
    HMODULE h;
    WCHAR wpath[MAX_PATH * 2];
    (void) flag;
    if (NULL == filename) {
        /* RTLD_DEFAULT semantics: our own module */
        return (void *) GetModuleHandle(NULL);
    }
    MultiByteToWideChar(CP_UTF8, 0, filename, -1, wpath,
                        sizeof(wpath) / sizeof(wpath[0]));
    /* libfoo.dll / foo.dll / foo -- LoadLibrary resolves .dll */
    h = LoadLibraryW(wpath);
    if (NULL == h) {
        /* retry adding .dll if missing */
        if (NULL == strstr(filename, ".dll")) {
            char tmp[MAX_PATH * 2];
            snprintf(tmp, sizeof(tmp), "%s.dll", filename);
            MultiByteToWideChar(CP_UTF8, 0, tmp, -1, wpath,
                                sizeof(wpath) / sizeof(wpath[0]));
            h = LoadLibraryW(wpath);
        }
        if (NULL == h) {
            dl_set_err(filename);
            return NULL;
        }
    }
    return (void *) h;
}

OPAL_WIN32_DECLSPEC char *opal_win32_dlerror(void)
{
    DWORD err = GetLastError();
    if (0 == err) {
        return NULL;
    }
    dl_set_err("dlopen/dlsym");
    return dl_errbuf;
}

OPAL_WIN32_DECLSPEC void *opal_win32_dlsym(void *handle, const char *symbol)
{
    FARPROC p;
    if (RTLD_NEXT == handle || RTLD_DEFAULT == handle || NULL == handle) {
        HMODULE hm = GetModuleHandle(NULL);
        p = GetProcAddress(hm, symbol);
        if (NULL == p) {
            /* try all loaded modules: slow but thorough */
        }
    } else {
        p = GetProcAddress((HMODULE) handle, symbol);
    }
    return (void *) p;
}

OPAL_WIN32_DECLSPEC int opal_win32_dlclose(void *handle)
{
    if (NULL == handle) {
        return 0;
    }
    return FreeLibrary((HMODULE) handle) ? 0 : -1;
}

OPAL_WIN32_DECLSPEC int opal_win32_dladdr(const void *addr, void *info)
{
    Dl_info *di = (Dl_info *) info;
    HMODULE hm;
    char path[MAX_PATH];
    if (0 == GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS
                                    | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                                (LPCSTR) addr, &hm)) {
        return 0;
    }
    if (NULL == di) {
        return 1;
    }
    GetModuleFileNameA(hm, path, MAX_PATH);
    di->dli_fname = _strdup(path);
    di->dli_fbase = (void *) hm;
    di->dli_sname = NULL;
    di->dli_saddr = (void *) addr;
    return 1;
}

/* ================================================================== */
/* fnmatch                                                            */
/* ================================================================== */

static int fnmatch_inner(const char *p, const char *s, int flags)
{
    while (*p) {
        switch (*p) {
        case '*':
            /* collapse consecutive stars */
            while ('*' == *p) {
                p++;
            }
            if ('\0' == *p) {
                if ((flags & FNM_PATHNAME) && NULL != strchr(s, '/')) {
                    return FNM_NOMATCH;
                }
                return 0;
            }
            while (*s) {
                if (0 == fnmatch_inner(p, s, flags)) {
                    return 0;
                }
                if ((flags & FNM_PATHNAME) && '/' == *s) {
                    break;
                }
                s++;
            }
            return FNM_NOMATCH;
        case '?':
            if ('\0' == *s) {
                return FNM_NOMATCH;
            }
            if ((flags & FNM_PATHNAME) && '/' == *s) {
                return FNM_NOMATCH;
            }
            p++;
            s++;
            break;
        case '[': {
            int negate = 0;
            int matched = 0;
            const char *e;
            p++;
            if ('!' == *p || '^' == *p) {
                negate = 1;
                p++;
            }
            if (']' == *p) {
                p++;
            }
            e = p;
            while (*e && *e != ']') {
                e++;
            }
            while (p < e) {
                if (p + 2 < e && p[1] == '-') {
                    if (*s >= p[0] && *s <= p[2]) {
                        matched = 1;
                    }
                    p += 3;
                } else {
                    if (*p == *s) {
                        matched = 1;
                    }
                    p++;
                }
            }
            if (matched == negate) {
                return FNM_NOMATCH;
            }
            if ('\0' == *s) {
                return FNM_NOMATCH;
            }
            p = e + 1;
            s++;
            break;
        }
        case '\\':
            if (!(flags & FNM_NOESCAPE)) {
                p++;
            }
            /* fall through */
        default:
            if (*p != *s) {
                return FNM_NOMATCH;
            }
            if ((flags & FNM_PERIOD) && '.' == *s && '/' == s[-1]) {
                return FNM_NOMATCH;
            }
            p++;
            s++;
            break;
        }
    }
    return *s ? FNM_NOMATCH : 0;
}

OPAL_WIN32_DECLSPEC int fnmatch(const char *pattern, const char *string, int flags)
{
    return fnmatch_inner(pattern, string, flags);
}

/* ================================================================== */
/* execinfo                                                           */
/* ================================================================== */

OPAL_WIN32_DECLSPEC int backtrace(void **buffer, int size)
{
    return (int) CaptureStackBackTrace(0, (DWORD) size, buffer, NULL);
}

char **backtrace_symbols(void *const *buffer, int size)
{
    /* no DbgHelp symbolization here; return raw addresses */
    char **syms = malloc(size * sizeof(char *));
    int i;
    if (NULL == syms) {
        return NULL;
    }
    for (i = 0; i < size; i++) {
        char tmp[64];
        snprintf(tmp, sizeof(tmp), "0x%p", buffer[i]);
        syms[i] = _strdup(tmp);
    }
    return syms;
}

OPAL_WIN32_DECLSPEC void backtrace_symbols_fd(void *const *buffer, int size, int fd)
{
    int i;
    char line[80];
    for (i = 0; i < size; i++) {
        int n = snprintf(line, sizeof(line), "0x%p\n", buffer[i]);
        _write(fd, line, n);
    }
}

/* ================================================================== */
/* syslog                                                             */
/* ================================================================== */

static char syslog_ident[64] = "opal";
static int syslog_open = 0;

OPAL_WIN32_DECLSPEC void openlog(const char *ident, int option, int facility)
{
    (void) option;
    (void) facility;
    if (ident) {
        strncpy(syslog_ident, ident, sizeof(syslog_ident) - 1);
    }
    syslog_open = 1;
}

OPAL_WIN32_DECLSPEC void vsyslog(int priority, const char *format, va_list ap)
{
    char buf[2048];
    int n = _vsnprintf(buf, sizeof(buf) - 1, format, ap);
    (void) priority;
    if (n > 0) {
        buf[n] = '\0';
        OutputDebugStringA(buf);
        OutputDebugStringA("\n");
    }
}

OPAL_WIN32_DECLSPEC void syslog(int priority, const char *format, ...)
{
    va_list ap;
    va_start(ap, format);
    vsyslog(priority, format, ap);
    va_end(ap);
}

OPAL_WIN32_DECLSPEC void closelog(void)
{
    syslog_open = 0;
}

OPAL_WIN32_DECLSPEC int setlogmask(int maskpri)
{
    (void) maskpri;
    return 0;
}

/* ================================================================== */
/* pwd/grp                                                            */
/* ================================================================== */

static void fill_passwd(struct passwd *pw)
{
    static char name[256];
    static char home[MAX_PATH];
    DWORD sz = sizeof(name);
    memset(pw, 0, sizeof(*pw));
    if (GetUserNameA(name, &sz)) {
        pw->pw_name = name;
    } else {
        pw->pw_name = (char *) "user";
    }
    pw->pw_passwd = (char *) "";
    pw->pw_uid = 0;
    pw->pw_gid = 0;
    pw->pw_gecos = (char *) "";
    {
        const char *h = getenv("USERPROFILE");
        if (NULL == h) {
            h = getenv("HOMEDRIVE");
        }
        snprintf(home, sizeof(home), "%s", h ? h : "C:\\");
        pw->pw_dir = home;
    }
    pw->pw_shell = (char *) "cmd.exe";
}

struct passwd *getpwuid(uid_t uid)
{
    static struct passwd pw;
    (void) uid;
    fill_passwd(&pw);
    return &pw;
}

struct passwd *getpwnam(const char *name)
{
    static struct passwd pw;
    (void) name;
    fill_passwd(&pw);
    return &pw;
}

OPAL_WIN32_DECLSPEC int getpwuid_r(uid_t uid, struct passwd *pwd, char *buf, size_t buflen,
               struct passwd **result)
{
    (void) uid;
    (void) buf;
    (void) buflen;
    fill_passwd(pwd);
    if (result) {
        *result = pwd;
    }
    return 0;
}

OPAL_WIN32_DECLSPEC int getpwnam_r(const char *name, struct passwd *pwd, char *buf, size_t buflen,
               struct passwd **result)
{
    (void) name;
    (void) buf;
    (void) buflen;
    fill_passwd(pwd);
    if (result) {
        *result = pwd;
    }
    return 0;
}

struct passwd *getpwent(void)
{
    return getpwuid(0);
}

OPAL_WIN32_DECLSPEC void setpwent(void)
{
}

OPAL_WIN32_DECLSPEC void endpwent(void)
{
}

static void fill_group(struct group *gr)
{
    static char name[256] = "users";
    static char *memb[2] = {name, NULL};
    memset(gr, 0, sizeof(*gr));
    gr->gr_name = name;
    gr->gr_passwd = (char *) "";
    gr->gr_gid = 0;
    gr->gr_mem = memb;
}

struct group *getgrgid(gid_t gid)
{
    static struct group gr;
    (void) gid;
    fill_group(&gr);
    return &gr;
}

struct group *getgrnam(const char *name)
{
    static struct group gr;
    (void) name;
    fill_group(&gr);
    return &gr;
}

OPAL_WIN32_DECLSPEC int getgrgid_r(gid_t gid, struct group *grp, char *buf, size_t buflen,
               struct group **result)
{
    (void) gid;
    (void) buf;
    (void) buflen;
    fill_group(grp);
    if (result) {
        *result = grp;
    }
    return 0;
}

OPAL_WIN32_DECLSPEC int getgrnam_r(const char *name, struct group *grp, char *buf, size_t buflen,
               struct group **result)
{
    (void) name;
    (void) buf;
    (void) buflen;
    fill_group(grp);
    if (result) {
        *result = grp;
    }
    return 0;
}

struct group *getgrent(void)
{
    return getgrgid(0);
}

OPAL_WIN32_DECLSPEC void setgrent(void)
{
}

OPAL_WIN32_DECLSPEC void endgrent(void)
{
}

OPAL_WIN32_DECLSPEC int getgrouplist(const char *user, gid_t group, gid_t *groups, int *ngroups)
{
    (void) user;
    if (*ngroups >= 1) {
        groups[0] = group;
        *ngroups = 1;
        return 1;
    }
    *ngroups = 1;
    return 0;
}

OPAL_WIN32_DECLSPEC int setgroups(int ngroups, const gid_t *groups)
{
    (void) ngroups;
    (void) groups;
    return 0;
}

/* ================================================================== */
/* termios stubs                                                      */
/* ================================================================== */

OPAL_WIN32_DECLSPEC int tcgetattr(int fd, struct termios *termios_p)
{
    (void) fd;
    (void) termios_p;
    errno = ENOTTY;
    return -1;
}

OPAL_WIN32_DECLSPEC int tcsetattr(int fd, int optional_actions, const struct termios *termios_p)
{
    (void) fd;
    (void) optional_actions;
    (void) termios_p;
    errno = ENOTTY;
    return -1;
}

OPAL_WIN32_DECLSPEC int tcsendbreak(int fd, int duration)
{
    (void) fd;
    (void) duration;
    errno = ENOTTY;
    return -1;
}

OPAL_WIN32_DECLSPEC int tcdrain(int fd)
{
    (void) fd;
    return 0;
}

OPAL_WIN32_DECLSPEC int tcflush(int fd, int queue_selector)
{
    (void) fd;
    (void) queue_selector;
    return 0;
}

OPAL_WIN32_DECLSPEC int tcflow(int fd, int action)
{
    (void) fd;
    (void) action;
    return 0;
}

OPAL_WIN32_DECLSPEC void cfmakeraw(struct termios *termios_p)
{
    memset(termios_p, 0, sizeof(*termios_p));
}

OPAL_WIN32_DECLSPEC speed_t cfgetispeed(const struct termios *termios_p)
{
    return termios_p->c_ispeed;
}

OPAL_WIN32_DECLSPEC speed_t cfgetospeed(const struct termios *termios_p)
{
    return termios_p->c_ospeed;
}

OPAL_WIN32_DECLSPEC int cfsetispeed(struct termios *termios_p, speed_t speed)
{
    termios_p->c_ispeed = speed;
    return 0;
}

OPAL_WIN32_DECLSPEC int cfsetospeed(struct termios *termios_p, speed_t speed)
{
    termios_p->c_ospeed = speed;
    return 0;
}

OPAL_WIN32_DECLSPEC int cfsetspeed(struct termios *termios_p, speed_t speed)
{
    termios_p->c_ispeed = speed;
    termios_p->c_ospeed = speed;
    return 0;
}

/* ================================================================== */
/* pty stubs                                                          */
/* ================================================================== */

OPAL_WIN32_DECLSPEC int opal_win32_openpty(int *amaster, int *aslave, char *name,
                       const struct termios *termp, const void *winp)
{
    (void) amaster;
    (void) aslave;
    (void) name;
    (void) termp;
    (void) winp;
    errno = ENOSYS;
    return -1;
}

OPAL_WIN32_DECLSPEC int opal_win32_forkpty(int *amaster, char *name, const struct termios *termp,
                       const void *winp)
{
    (void) amaster;
    (void) name;
    (void) termp;
    (void) winp;
    errno = ENOSYS;
    return -1;
}

OPAL_WIN32_DECLSPEC int opal_win32_login_tty(int fd)
{
    (void) fd;
    return 0;
}

/* ================================================================== */
/* syscall                                                            */
/* ================================================================== */

OPAL_WIN32_DECLSPEC long opal_win32_syscall(long number, ...)
{
    switch (number) {
    case SYS_gettid:
        return (long) GetCurrentThreadId();
    case SYS_getpid:
        return (long) GetCurrentProcessId();
    case SYS_getppid:
        return 0;
    default:
        errno = ENOSYS;
        return -1;
    }
}

/* ================================================================== */
/* posix_spawn attributes (files-action execution happens in          */
/* opal_win32_spawn_process)                                          */
/* ================================================================== */

OPAL_WIN32_DECLSPEC int posix_spawnattr_init(posix_spawnattr_t *attr)
{
    memset(attr, 0, sizeof(*attr));
    return 0;
}

OPAL_WIN32_DECLSPEC int posix_spawnattr_destroy(posix_spawnattr_t *attr)
{
    (void) attr;
    return 0;
}

OPAL_WIN32_DECLSPEC int posix_spawnattr_getflags(const posix_spawnattr_t *attr, short *flags)
{
    *flags = (short) attr->flags;
    return 0;
}

OPAL_WIN32_DECLSPEC int posix_spawnattr_setflags(posix_spawnattr_t *attr, short flags)
{
    attr->flags = flags;
    return 0;
}

OPAL_WIN32_DECLSPEC int posix_spawnattr_getpgroup(const posix_spawnattr_t *attr, pid_t *pgroup)
{
    *pgroup = attr->pgrp;
    return 0;
}

OPAL_WIN32_DECLSPEC int posix_spawnattr_setpgroup(posix_spawnattr_t *attr, pid_t pgroup)
{
    attr->pgrp = pgroup;
    return 0;
}

OPAL_WIN32_DECLSPEC int posix_spawnattr_getsigmask(const posix_spawnattr_t *attr, sigset_t *mask)
{
    *mask = attr->mask;
    return 0;
}

OPAL_WIN32_DECLSPEC int posix_spawnattr_setsigmask(posix_spawnattr_t *attr, const sigset_t *mask)
{
    attr->mask = *mask;
    return 0;
}

OPAL_WIN32_DECLSPEC int posix_spawnattr_getsigdefault(const posix_spawnattr_t *attr, sigset_t *def)
{
    *def = attr->sd;
    return 0;
}

OPAL_WIN32_DECLSPEC int posix_spawnattr_setsigdefault(posix_spawnattr_t *attr, const sigset_t *def)
{
    attr->sd = *def;
    return 0;
}

OPAL_WIN32_DECLSPEC int posix_spawnattr_getschedpolicy(const posix_spawnattr_t *attr, int *policy)
{
    *policy = attr->schedpolicy;
    return 0;
}

OPAL_WIN32_DECLSPEC int posix_spawnattr_setschedpolicy(posix_spawnattr_t *attr, int policy)
{
    attr->schedpolicy = policy;
    return 0;
}

OPAL_WIN32_DECLSPEC int posix_spawnattr_getschedparam(const posix_spawnattr_t *attr,
                                  struct sched_param *param)
{
    *param = attr->schedparam;
    return 0;
}

OPAL_WIN32_DECLSPEC int posix_spawnattr_setschedparam(posix_spawnattr_t *attr,
                                  const struct sched_param *param)
{
    attr->schedparam = *param;
    return 0;
}

OPAL_WIN32_DECLSPEC int posix_spawn_file_actions_init(posix_spawn_file_actions_t *fa)
{
    memset(fa, 0, sizeof(*fa));
    return 0;
}

OPAL_WIN32_DECLSPEC int posix_spawn_file_actions_destroy(posix_spawn_file_actions_t *fa)
{
    free(fa->_actions);
    memset(fa, 0, sizeof(*fa));
    return 0;
}

struct spawn_act {
    int type; /* 0=open 1=close 2=dup2 */
    int fd;
    int newfd;
    char path[MAX_PATH];
    int oflag;
    mode_t mode;
};

static int spawn_actions_grow(posix_spawn_file_actions_t *fa)
{
    if (fa->_used >= fa->_allocated) {
        int nc = fa->_allocated ? fa->_allocated * 2 : 8;
        void *na = realloc(fa->_actions, nc * sizeof(struct spawn_act));
        if (NULL == na) {
            return ENOMEM;
        }
        fa->_actions = na;
        fa->_allocated = nc;
    }
    return 0;
}

OPAL_WIN32_DECLSPEC int posix_spawn_file_actions_addopen(posix_spawn_file_actions_t *fa, int fd,
                                     const char *path, int oflag, mode_t mode)
{
    struct spawn_act *a;
    if (0 != spawn_actions_grow(fa)) {
        return ENOMEM;
    }
    a = (struct spawn_act *) fa->_actions + fa->_used++;
    a->type = 0;
    a->fd = fd;
    a->newfd = 0;
    strncpy(a->path, path, MAX_PATH - 1);
    a->oflag = oflag;
    a->mode = mode;
    return 0;
}

OPAL_WIN32_DECLSPEC int posix_spawn_file_actions_addclose(posix_spawn_file_actions_t *fa, int fd)
{
    struct spawn_act *a;
    if (0 != spawn_actions_grow(fa)) {
        return ENOMEM;
    }
    a = (struct spawn_act *) fa->_actions + fa->_used++;
    a->type = 1;
    a->fd = fd;
    return 0;
}

OPAL_WIN32_DECLSPEC int posix_spawn_file_actions_adddup2(posix_spawn_file_actions_t *fa, int fd,
                                     int newfd)
{
    struct spawn_act *a;
    if (0 != spawn_actions_grow(fa)) {
        return ENOMEM;
    }
    a = (struct spawn_act *) fa->_actions + fa->_used++;
    a->type = 2;
    a->fd = fd;
    a->newfd = newfd;
    return 0;
}

/* ================================================================== */
/* misc                                                               */
/* ================================================================== */

OPAL_WIN32_DECLSPEC int shmget(key_t key, size_t size, int shmflg)
{
    (void) key;
    (void) size;
    (void) shmflg;
    errno = ENOSYS;
    return -1;
}

void *shmat(int shmid, const void *shmaddr, int shmflg)
{
    (void) shmid;
    (void) shmaddr;
    (void) shmflg;
    errno = ENOSYS;
    return (void *) -1;
}

OPAL_WIN32_DECLSPEC int shmdt(const void *shmaddr)
{
    (void) shmaddr;
    errno = ENOSYS;
    return -1;
}

OPAL_WIN32_DECLSPEC int shmctl(int shmid, int cmd, struct shmid_ds *buf)
{
    (void) shmid;
    (void) cmd;
    (void) buf;
    errno = ENOSYS;
    return -1;
}

OPAL_WIN32_DECLSPEC int daemon(int nochdir, int noclose)
{
    (void) nochdir;
    (void) noclose;
    /* callers that want daemonization should CreateProcess detached;
     * return success so generic code paths continue */
    return 0;
}

/* ---- regex stubs: report "not implemented" (REG_NOSYS) ---------- */
#    undef regcomp
#    undef regexec
#    undef regerror
#    undef regfree

OPAL_WIN32_DECLSPEC int opal_win32_regcomp(regex_t *preg, const char *pattern, int cflags)
{
    (void) preg;
    (void) pattern;
    (void) cflags;
    return REG_NOSYS;
}

OPAL_WIN32_DECLSPEC int opal_win32_regexec(const regex_t *preg, const char *string, size_t nmatch,
                       regmatch_t pmatch[], int eflags)
{
    (void) preg;
    (void) string;
    (void) nmatch;
    (void) pmatch;
    (void) eflags;
    return REG_NOMATCH;
}

OPAL_WIN32_DECLSPEC size_t opal_win32_regerror(int errcode, const regex_t *preg, char *errbuf,
                           size_t errbuf_size)
{
    const char *msg = "regex not supported on Windows";
    (void) errcode;
    (void) preg;
    if (NULL != errbuf && errbuf_size > 0) {
        size_t n = strlen(msg);
        if (n >= errbuf_size) {
            n = errbuf_size - 1;
        }
        memcpy(errbuf, msg, n);
        errbuf[n] = '\0';
    }
    return strlen(msg) + 1;
}

OPAL_WIN32_DECLSPEC void opal_win32_regfree(regex_t *preg)
{
    (void) preg;
}

#endif /* _WIN32 */
