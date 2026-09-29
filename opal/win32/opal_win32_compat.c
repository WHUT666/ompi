/*
 * Copyright (c) 2026      Amazon.com, Inc. or its affiliates.
 *                         All Rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 *
 * Core POSIX compat implementations for the native Windows build:
 * unistd/env/string/time helpers.  Everything here is plain Win32.
 */

#include "opal_config.h"

#ifdef _WIN32

#    include "opal/win32/opal_win32.h"

/* impl translation unit: undef every macro we implement here */
#    undef getpid
#    undef pipe
#    undef pipe2
#    undef gethostname
#    undef getpagesize
#    undef sysconf
#    undef sleep
#    undef usleep
#    undef nanosleep
#    undef truncate
#    undef ftruncate
#    undef getlogin
#    undef getlogin_r
#    undef link
#    undef symlink
#    undef readlink
#    undef fcntl
#    undef ioctl
#    undef mkstemp
#    undef mkstemps
#    undef mkdtemp
#    undef pread
#    undef pwrite
#    undef readv
#    undef writev
#    undef setenv
#    undef unsetenv
#    undef gettimeofday
#    undef getitimer
#    undef setitimer
#    undef utimes
#    undef utimensat
#    undef clock_gettime
#    undef clock_getres
#    undef clock_settime
#    undef nanosleep
#    undef localtime_r
#    undef gmtime_r
#    undef asctime_r
#    undef ctime_r
#    undef getopt
#    undef close
#    undef mkdir
#    undef stat
#    undef lstat
#    undef fstat
#    undef fchmod
#    undef access
#    undef strerror_r
#    undef strtok_r
#    undef strndup
#    undef strsep
#    undef strlcpy
#    undef strlcat
#    undef posix_memalign
#    undef realpath
#    undef getline
#    undef getdelim
#    undef dprintf
#    undef flock
#    undef uname
#    undef times
#    undef popen
#    undef pclose
#    undef symlinkat
#    undef linkat
#    undef getsubopt
#    undef daemon
#    undef random
#    undef srandom
#    undef getloadavg
#    undef execl
#    undef execle
#    undef execlp
#    undef execv
#    undef execve
#    undef execvp
#    undef execlpe
#    undef fork
#    undef vfork

/* ================================================================== */
/* time                                                               */
/* ================================================================== */

static ULONGLONG opal_win32_unix_epoch_100ns = 116444736000000000ULL;

OPAL_WIN32_DECLSPEC int opal_win32_gettimeofday(struct timeval *tv, void *tz)
{
    FILETIME ft;
    ULONGLONG t;
    (void) tz;
    GetSystemTimePreciseAsFileTime(&ft);
    t = ((ULONGLONG) ft.dwHighDateTime << 32) | ft.dwLowDateTime;
    if (tv) {
        t -= opal_win32_unix_epoch_100ns;
        tv->tv_sec = (long) (t / 10000000ULL);
        tv->tv_usec = (long) ((t % 10000000ULL) / 10);
    }
    return 0;
}

OPAL_WIN32_DECLSPEC int clock_gettime(int clock_id, struct timespec *ts)
{
    static LARGE_INTEGER freq = {0};
    LARGE_INTEGER now;
    FILETIME ft;
    ULONGLONG t;

    if (0 == freq.QuadPart) {
        QueryPerformanceFrequency(&freq);
    }
    switch (clock_id) {
    case CLOCK_REALTIME:
        GetSystemTimePreciseAsFileTime(&ft);
        t = ((ULONGLONG) ft.dwHighDateTime << 32) | ft.dwLowDateTime;
        t -= opal_win32_unix_epoch_100ns;
        ts->tv_sec = (time_t) (t / 10000000ULL);
        ts->tv_nsec = (long) ((t % 10000000ULL) * 100);
        return 0;
    case CLOCK_MONOTONIC:
    case CLOCK_MONOTONIC_RAW:
    case CLOCK_MONOTONIC_COARSE:
        QueryPerformanceCounter(&now);
        ts->tv_sec = (time_t) (now.QuadPart / freq.QuadPart);
        ts->tv_nsec = (long) (((now.QuadPart % freq.QuadPart) * 1000000000LL)
                              / freq.QuadPart);
        return 0;
    case CLOCK_PROCESS_CPUTIME_ID:
    case CLOCK_THREAD_CPUTIME_ID: {
        FILETIME c, e, k, u;
        HANDLE h = (CLOCK_PROCESS_CPUTIME_ID == clock_id)
                       ? GetCurrentProcess()
                       : GetCurrentThread();
        if (0 == GetProcessTimes(h, &c, &e, &k, &u)) {
            if (CLOCK_THREAD_CPUTIME_ID == clock_id) {
                if (0 == GetThreadTimes(h, &c, &e, &k, &u)) {
                    return -1;
                }
            } else {
                return -1;
            }
        }
        t = (((ULONGLONG) u.dwHighDateTime << 32) | u.dwLowDateTime)
            + (((ULONGLONG) k.dwHighDateTime << 32) | k.dwLowDateTime);
        ts->tv_sec = (time_t) (t / 10000000ULL);
        ts->tv_nsec = (long) ((t % 10000000ULL) * 100);
        return 0;
    }
    default:
        errno = EINVAL;
        return -1;
    }
}

OPAL_WIN32_DECLSPEC int clock_getres(int clock_id, struct timespec *res)
{
    (void) clock_id;
    if (res) {
        res->tv_sec = 0;
        res->tv_nsec = 100; /* ~100ns resolution from QPC/PreciseFileTime */
    }
    return 0;
}

OPAL_WIN32_DECLSPEC int clock_settime(int clock_id, const struct timespec *ts)
{
    (void) clock_id;
    (void) ts;
    errno = EPERM;
    return -1;
}

OPAL_WIN32_DECLSPEC int nanosleep(const struct timespec *req, struct timespec *rem)
{
    (void) rem;
    /* Sleep() granularity is ~15.6ms by default; for sub-millisecond
     * waits use a waitable timer. */
    LONGLONG ns = req->tv_sec * 1000000000LL + req->tv_nsec;
    if (ns >= 1000000LL) {
        DWORD ms = (DWORD) ((ns + 999999LL) / 1000000LL);
        Sleep(ms);
    } else {
        /* spin for short sleeps */
        LARGE_INTEGER freq, start, now;
        LONGLONG ticks;
        QueryPerformanceFrequency(&freq);
        ticks = (LONGLONG) ((ns * freq.QuadPart) / 1000000000LL);
        QueryPerformanceCounter(&start);
        do {
            QueryPerformanceCounter(&now);
        } while (now.QuadPart - start.QuadPart < ticks);
    }
    return 0;
}

OPAL_WIN32_DECLSPEC unsigned int opal_win32_sleep(unsigned int seconds)
{
    Sleep(seconds * 1000);
    return 0;
}

OPAL_WIN32_DECLSPEC int opal_win32_usleep(useconds_t usec)
{
    struct timespec ts;
    ts.tv_sec = usec / 1000000;
    ts.tv_nsec = (long) (usec % 1000000) * 1000;
    return nanosleep(&ts, NULL);
}

struct tm *localtime_r(const time_t *timep, struct tm *result)
{
    if (0 != localtime_s(result, timep)) {
        return NULL;
    }
    return result;
}

struct tm *gmtime_r(const time_t *timep, struct tm *result)
{
    if (0 != gmtime_s(result, timep)) {
        return NULL;
    }
    return result;
}

char *asctime_r(const struct tm *tm, char *buf)
{
    if (0 != asctime_s(buf, 26, tm)) {
        return NULL;
    }
    return buf;
}

char *ctime_r(const time_t *timep, char *buf)
{
    if (0 != ctime_s(buf, 26, timep)) {
        return NULL;
    }
    return buf;
}

OPAL_WIN32_DECLSPEC time_t timegm(struct tm *tm)
{
    return _mkgmtime(tm);
}

char *strptime(const char *buf, const char *format, struct tm *tm)
{
    /* minimal strptime: handles %Y-%m-%d %H:%M:%S and ISO8601-ish */
    int y = 0, mo = 0, d = 0, h = 0, mi = 0, s = 0;
    (void) format;
    if (6 <= sscanf_s(buf, "%d-%d-%dT%d:%d:%d", &y, &mo, &d, &h, &mi, &s)
        || 6 <= sscanf_s(buf, "%d-%d-%d %d:%d:%d", &y, &mo, &d, &h, &mi, &s)) {
        tm->tm_year = y - 1900;
        tm->tm_mon = mo - 1;
        tm->tm_mday = d;
        tm->tm_hour = h;
        tm->tm_min = mi;
        tm->tm_sec = s;
        tm->tm_isdst = -1;
        return (char *) (buf + strlen(buf));
    }
    return NULL;
}

OPAL_WIN32_DECLSPEC int opal_win32_getitimer(int which, struct itimerval *value)
{
    (void) which;
    memset(value, 0, sizeof(*value));
    return 0;
}

OPAL_WIN32_DECLSPEC int opal_win32_setitimer(int which, const struct itimerval *value,
                         struct itimerval *ovalue)
{
    (void) which;
    if (ovalue) {
        memset(ovalue, 0, sizeof(*ovalue));
    }
    if (value && (value->it_value.tv_sec || value->it_value.tv_usec)) {
        /* one-shot scheduling is not wired up; treat as success so
         * profiling paths don't abort */
    }
    return 0;
}

OPAL_WIN32_DECLSPEC int opal_win32_utimes(const char *path, const struct timeval times[2])
{
    struct utimbuf tb;
    if (times) {
        tb.actime = (time_t) times[0].tv_sec;
        tb.modtime = (time_t) times[1].tv_sec;
        return _utime64(path, (struct __utimbuf64 *) &tb);
    }
    return _utime64(path, NULL);
}

OPAL_WIN32_DECLSPEC int utimensat(int dirfd, const char *path, const struct timespec times[2],
              int flags)
{
    struct utimbuf tb;
    char full[MAX_PATH];
    (void) flags;
    if (0 != opal_win32_resolve_at(dirfd, path, full, sizeof(full))) {
        return -1;
    }
    if (times) {
        tb.actime = times[0].tv_sec;
        tb.modtime = times[1].tv_sec;
        return _utime64(full, (struct __utimbuf64 *) &tb);
    }
    return _utime64(full, NULL);
}

/* ================================================================== */
/* unistd                                                             */
/* ================================================================== */

OPAL_WIN32_DECLSPEC int opal_win32_pipe2(int fds[2], int flags)
{
    int cflags = _O_BINARY;
    if (flags & O_CLOEXEC) {
        cflags |= _O_NOINHERIT;
    }
    if (0 != _pipe(fds, 4096, cflags)) {
        return -1;
    }
    if (flags & O_NONBLOCK) {
        /* NT pipes support nonblocking via named-pipe handles; the CRT
         * anonymous pipe handle cannot be switched, so this is a
         * documented gap -- callers that need nonblocking pipes should
         * use sockets. */
    }
    return 0;
}

OPAL_WIN32_DECLSPEC int opal_win32_getpagesize(void)
{
    static long ps = 0;
    if (0 == ps) {
        SYSTEM_INFO si;
        GetSystemInfo(&si);
        ps = (long) si.dwPageSize;
    }
    return (int) ps;
}

OPAL_WIN32_DECLSPEC long opal_win32_sysconf(int name)
{
    SYSTEM_INFO si;
    switch (name) {
    case _SC_PAGESIZE:
        return opal_win32_getpagesize();
    case _SC_NPROCESSORS_CONF:
    case _SC_NPROCESSORS_ONLN:
        GetSystemInfo(&si);
        return (long) si.dwNumberOfProcessors;
    case _SC_OPEN_MAX:
        return 2048;
    case _SC_ARG_MAX:
        return 32767;
    case _SC_CLK_TCK:
        return 1000;
    case _SC_PHYS_PAGES: {
        MEMORYSTATUSEX ms;
        ms.dwLength = sizeof(ms);
        GlobalMemoryStatusEx(&ms);
        return (long) (ms.ullTotalPhys / opal_win32_getpagesize());
    }
    case _SC_AVPHYS_PAGES: {
        MEMORYSTATUSEX ms;
        ms.dwLength = sizeof(ms);
        GlobalMemoryStatusEx(&ms);
        return (long) (ms.ullAvailPhys / opal_win32_getpagesize());
    }
    case _SC_HOST_NAME_MAX:
        return 256;
    case _SC_LOGIN_NAME_MAX:
        return 256;
    case _SC_NGROUPS_MAX:
        return 16;
    case _SC_CHILD_MAX:
        return 128;
    case _SC_IOV_MAX:
        return 1024;
    case _SC_VERSION:
        return 200809L;
    default:
        errno = EINVAL;
        return -1;
    }
}

OPAL_WIN32_DECLSPEC int opal_win32_truncate(const char *path, off_t length)
{
    int fd, rc;
    fd = _open(path, _O_RDWR | _O_BINARY);
    if (fd < 0) {
        return -1;
    }
    rc = _chsize_s(fd, length);
    _close(fd);
    return rc;
}

OPAL_WIN32_DECLSPEC int opal_win32_ftruncate(int fd, off_t length)
{
    return _chsize_s(fd, length);
}

OPAL_WIN32_DECLSPEC int opal_win32_link(const char *oldpath, const char *newpath)
{
    WCHAR wold[MAX_PATH], wnew[MAX_PATH];
    MultiByteToWideChar(CP_UTF8, 0, oldpath, -1, wold, MAX_PATH);
    MultiByteToWideChar(CP_UTF8, 0, newpath, -1, wnew, MAX_PATH);
    return CreateHardLinkW(wnew, wold, NULL) ? 0 : -1;
}

OPAL_WIN32_DECLSPEC int opal_win32_symlink(const char *oldpath, const char *newpath)
{
    WCHAR wold[MAX_PATH], wnew[MAX_PATH];
    DWORD flags = 0x2 /* SYMBOLIC_LINK_FLAG_ALLOW_UNPRIVILEGED_CREATE */;
    DWORD attr;
    MultiByteToWideChar(CP_UTF8, 0, oldpath, -1, wold, MAX_PATH);
    MultiByteToWideChar(CP_UTF8, 0, newpath, -1, wnew, MAX_PATH);
    attr = GetFileAttributesW(wold);
    if (attr != INVALID_FILE_ATTRIBUTES && (attr & FILE_ATTRIBUTE_DIRECTORY)) {
        flags |= SYMBOLIC_LINK_FLAG_DIRECTORY;
    }
    return CreateSymbolicLinkW(wnew, wold, flags) ? 0 : -1;
}

OPAL_WIN32_DECLSPEC ssize_t opal_win32_readlink(const char *path, char *buf, size_t bufsize)
{
    /* Return the fully resolved path of a reparse point.  Uses
     * GetFinalPathNameByHandle (the resolved target), which is what
     * callers here actually want. */
    HANDLE h;
    WCHAR wpath[MAX_PATH];
    WCHAR wfinal[4096];
    DWORD n;
    char utf8[4096];
    int un;
    const char *resolved = utf8;
    size_t rlen;

    MultiByteToWideChar(CP_UTF8, 0, path, -1, wpath, MAX_PATH);
    h = CreateFileW(wpath, 0, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                    NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);
    if (INVALID_HANDLE_VALUE == h) {
        errno = ENOENT;
        return -1;
    }
    n = GetFinalPathNameByHandleW(h, wfinal, 4096, VOLUME_NAME_DOS);
    CloseHandle(h);
    if (0 == n || n >= 4096) {
        errno = EIO;
        return -1;
    }
    un = WideCharToMultiByte(CP_UTF8, 0, wfinal, -1, utf8, sizeof(utf8), NULL, NULL);
    if (un <= 0) {
        errno = EIO;
        return -1;
    }
    /* strip the "\\?\" prefix that GetFinalPathNameByHandle adds */
    if (0 == strncmp(utf8, "\\\\?\\", 4)) {
        resolved = utf8 + 4;
    }
    rlen = strlen(resolved);
    if (rlen >= bufsize) {
        errno = ENAMETOOLONG;
        return -1;
    }
    memcpy(buf, resolved, rlen + 1);
    return (ssize_t) rlen;
}

OPAL_WIN32_DECLSPEC char *opal_win32_getlogin(void)
{
    static char name[256];
    DWORD sz = sizeof(name);
    if (0 == GetUserNameA(name, &sz)) {
        return NULL;
    }
    return name;
}

OPAL_WIN32_DECLSPEC int opal_win32_getlogin_r(char *buf, size_t bufsize)
{
    DWORD sz = (DWORD) bufsize;
    if (0 == GetUserNameA(buf, &sz)) {
        return EINVAL;
    }
    return 0;
}

OPAL_WIN32_DECLSPEC ssize_t opal_win32_pread(int fd, void *buf, size_t count, off_t offset)
{
    __int64 saved = _lseeki64(fd, 0, SEEK_CUR);
    ssize_t rc;
    if (saved < 0 || _lseeki64(fd, offset, SEEK_SET) < 0) {
        return -1;
    }
    rc = _read(fd, buf, (unsigned int) count);
    _lseeki64(fd, saved, SEEK_SET);
    return rc;
}

OPAL_WIN32_DECLSPEC ssize_t opal_win32_pwrite(int fd, const void *buf, size_t count, off_t offset)
{
    __int64 saved = _lseeki64(fd, 0, SEEK_CUR);
    ssize_t rc;
    if (saved < 0 || _lseeki64(fd, offset, SEEK_SET) < 0) {
        return -1;
    }
    rc = _write(fd, buf, (unsigned int) count);
    _lseeki64(fd, saved, SEEK_SET);
    return rc;
}

OPAL_WIN32_DECLSPEC ssize_t opal_win32_readv(int fd, const struct iovec *iov, int iovcnt)
{
    ssize_t total = 0;
    int i;
    for (i = 0; i < iovcnt; i++) {
        /* route through opal_win32_read so socket fds go to recv() and
         * nonblocking pipes get the PeekNamedPipe guard - a bare _read()
         * on either is EBADF or a blocking hang */
        ssize_t rc = opal_win32_read(fd, iov[i].iov_base, iov[i].iov_len);
        if (rc < 0) {
            return (total > 0) ? total : rc;
        }
        total += rc;
        if ((size_t) rc < iov[i].iov_len) {
            break;
        }
    }
    return total;
}

OPAL_WIN32_DECLSPEC ssize_t opal_win32_writev(int fd, const struct iovec *iov, int iovcnt)
{
    ssize_t total = 0;
    int i;
    for (i = 0; i < iovcnt; i++) {
        /* same: opal_win32_write sends on sockets; _write() cannot */
        ssize_t rc = opal_win32_write(fd, iov[i].iov_base, iov[i].iov_len);
        if (rc < 0) {
            return (total > 0) ? total : rc;
        }
        total += rc;
        if ((size_t) rc < iov[i].iov_len) {
            break;
        }
    }
    return total;
}

OPAL_WIN32_DECLSPEC ssize_t opal_win32_process_vm_readv(pid_t pid, const struct iovec *lvec,
                                    unsigned long liovcnt, const struct iovec *rvec,
                                    unsigned long riovcnt, unsigned long flags)
{
    HANDLE h;
    SIZE_T total = 0;
    unsigned long li = 0, ri = 0;
    SIZE_T loff = 0, roff = 0;
    (void) flags;

    h = OpenProcess(PROCESS_VM_READ | PROCESS_QUERY_INFORMATION, FALSE, (DWORD) pid);
    if (NULL == h) {
        errno = ESRCH;
        return -1;
    }
    while (ri < riovcnt && li < liovcnt) {
        SIZE_T lremain = lvec[li].iov_len - loff;
        SIZE_T rremain = rvec[ri].iov_len - roff;
        SIZE_T chunk = (lremain < rremain) ? lremain : rremain;
        SIZE_T nread = 0;
        if (chunk > 0) {
            if (0 == ReadProcessMemory(h, (char *) rvec[ri].iov_base + roff,
                                       (char *) lvec[li].iov_base + loff, chunk,
                                       &nread)) {
                CloseHandle(h);
                errno = EFAULT;
                return (total > 0) ? (ssize_t) total : -1;
            }
            loff += nread;
            roff += nread;
            total += nread;
        }
        if (loff >= lvec[li].iov_len) {
            li++;
            loff = 0;
        }
        if (roff >= rvec[ri].iov_len) {
            ri++;
            roff = 0;
        }
        if (0 == chunk) {
            break;
        }
    }
    CloseHandle(h);
    return (ssize_t) total;
}

OPAL_WIN32_DECLSPEC ssize_t opal_win32_process_vm_writev(pid_t pid, const struct iovec *lvec,
                                     unsigned long liovcnt, const struct iovec *rvec,
                                     unsigned long riovcnt, unsigned long flags)
{
    HANDLE h;
    SIZE_T total = 0;
    unsigned long li = 0, ri = 0;
    SIZE_T loff = 0, roff = 0;
    (void) flags;

    h = OpenProcess(PROCESS_VM_WRITE | PROCESS_VM_OPERATION
                        | PROCESS_QUERY_INFORMATION,
                    FALSE, (DWORD) pid);
    if (NULL == h) {
        errno = ESRCH;
        return -1;
    }
    while (ri < riovcnt && li < liovcnt) {
        SIZE_T lremain = lvec[li].iov_len - loff;
        SIZE_T rremain = rvec[ri].iov_len - roff;
        SIZE_T chunk = (lremain < rremain) ? lremain : rremain;
        SIZE_T nwritten = 0;
        if (chunk > 0) {
            if (0 == WriteProcessMemory(h, (char *) rvec[ri].iov_base + roff,
                                        (char *) lvec[li].iov_base + loff, chunk,
                                        &nwritten)) {
                CloseHandle(h);
                errno = EFAULT;
                return (total > 0) ? (ssize_t) total : -1;
            }
            loff += nwritten;
            roff += nwritten;
            total += nwritten;
        }
        if (loff >= lvec[li].iov_len) {
            li++;
            loff = 0;
        }
        if (roff >= rvec[ri].iov_len) {
            ri++;
            roff = 0;
        }
        if (0 == chunk) {
            break;
        }
    }
    CloseHandle(h);
    return (ssize_t) total;
}

/* O_NONBLOCK tracking for non-socket CRT fds.  Winsock fds are flipped
 * through ioctlsocket(FIONBIO) directly; for pipe fds we record the flag
 * here and let opal_win32_read PeekNamedPipe-guard reads on them. */
static unsigned char nb_fd_table[8192]; /* _getmaxstdio bound */
static CRITICAL_SECTION nb_fd_lock;
static volatile LONG nb_fd_lock_init = 0;

static void nb_fd_lock_once(void)
{
    if (0 == InterlockedCompareExchange(&nb_fd_lock_init, 1, 0)) {
        InitializeCriticalSection(&nb_fd_lock);
        InterlockedExchange(&nb_fd_lock_init, 2);
    }
    while (1 == nb_fd_lock_init) {
        Sleep(0);
    }
}

OPAL_WIN32_DECLSPEC void opal_win32_fd_set_nonblocking(int fd, int nb)
{
    nb_fd_lock_once();
    if (fd < 0 || fd >= (int) (sizeof(nb_fd_table))) {
        return;
    }
    EnterCriticalSection(&nb_fd_lock);
    nb_fd_table[fd] = nb ? 1 : 0;
    LeaveCriticalSection(&nb_fd_lock);
}

OPAL_WIN32_DECLSPEC int opal_win32_fd_is_nonblocking(int fd)
{
    int v;
    nb_fd_lock_once();
    if (fd < 0 || fd >= (int) (sizeof(nb_fd_table))) {
        return 0;
    }
    EnterCriticalSection(&nb_fd_lock);
    v = nb_fd_table[fd];
    LeaveCriticalSection(&nb_fd_lock);
    return v;
}

OPAL_WIN32_DECLSPEC int opal_win32_fcntl(int fd, int cmd, ...)
{
    va_list ap;
    intptr_t arg;
    int rc = 0;
    va_start(ap, cmd);
    /* Win64 varargs occupy 8-byte slots: reading intptr_t is safe for
     * int args and does not truncate pointer args (F_SETLK takes a
     * struct flock *).  Reading int here would lose the high 32 bits
     * of a pointer. */
    arg = va_arg(ap, intptr_t);
    va_end(ap);

    switch (cmd) {
    case F_GETFD:
        return 0;
    case F_SETFD:
        return 0;
    case F_GETFL: {
        /* CRT fds do not record mode well; be generous */
        int fl = O_RDWR;
        if (opal_win32_fd_is_nonblocking(fd)) {
            fl |= O_NONBLOCK;
        }
        return fl;
    }
    case F_SETFL: {
        if (arg & O_NONBLOCK) {
            /* if this fd is a socket, flip it to nonblocking */
            if (opal_win32_is_socket(fd)) {
                u_long nb = 1;
                if (0 != ioctlsocket((SOCKET) fd, FIONBIO, &nb)) {
                    errno = opal_win32_socket_errno();
                    return -1;
                }
            }
            /* record the flag for sockets and pipes alike so F_GETFL can
             * report O_NONBLOCK: Windows has no way to query a socket's
             * blocking mode back, and callers (e.g. ptl's blocking recv)
             * branch on F_GETFL to tell an SO_RCVTIMEO expiry from a real
             * nonblocking EAGAIN */
            opal_win32_fd_set_nonblocking(fd, 1);
        } else {
            if (opal_win32_is_socket(fd)) {
                u_long nb = 0;
                if (0 != ioctlsocket((SOCKET) fd, FIONBIO, &nb)) {
                    errno = opal_win32_socket_errno();
                    return -1;
                }
            }
            opal_win32_fd_set_nonblocking(fd, 0);
        }
        return 0;
    }
    case F_DUPFD:
    case F_DUPFD_CLOEXEC:
        return _dup(fd);
    case F_GETLK:
    case F_SETLK:
    case F_SETLKW: {
        /* POSIX byte-range locking over LockFileEx.  LockFileEx takes
         * the offset through an OVERLAPPED and the length separately;
         * l_len == 0 means "to EOF" in POSIX, which maps to the
         * largest possible extent so that a later F_UNLCK computed the
         * same way still matches the locked range. */
        struct flock *lk = (struct flock *) arg;
        HANDLE h;
        __int64 base = 0, start;
        DWORD nlo, nhi, flags;
        OVERLAPPED ov;

        if (NULL == lk) {
            errno = EFAULT;
            return -1;
        }
        h = (HANDLE) _get_osfhandle(fd);
        if (INVALID_HANDLE_VALUE == h || NULL == h) {
            errno = EBADF;
            return -1;
        }
        switch (lk->l_whence) {
        case SEEK_SET:
            base = 0;
            break;
        case SEEK_CUR:
            base = _lseeki64(fd, 0, SEEK_CUR);
            break;
        case SEEK_END:
            base = _filelengthi64(fd);
            break;
        default:
            errno = EINVAL;
            return -1;
        }
        if (base < 0) {
            return -1;
        }
        start = base + lk->l_start;
        if (0 == lk->l_len) {
            nlo = 0xFFFFFFFF;
            nhi = 0xFFFFFFFF;
        } else {
            nlo = (DWORD) (lk->l_len & 0xFFFFFFFF);
            nhi = (DWORD) (((unsigned __int64) lk->l_len) >> 32);
        }
        memset(&ov, 0, sizeof(ov));
        ov.Offset = (DWORD) (start & 0xFFFFFFFF);
        ov.OffsetHigh = (DWORD) (((unsigned __int64) start) >> 32);

        if (F_UNLCK == lk->l_type) {
            if (!UnlockFileEx(h, 0, nlo, nhi, &ov)) {
                errno = EIO;
                return -1;
            }
            return 0;
        }
        flags = (F_WRLCK == lk->l_type) ? LOCKFILE_EXCLUSIVE_LOCK : 0;
        if (F_GETLK == cmd) {
            /* Probe: if a conflicting lock were held the immediate lock
             * would fail.  There is no l_pid to report on Windows;
             * leaving l_type set signals "would block". */
            if (LockFileEx(h, flags | LOCKFILE_FAIL_IMMEDIATELY, 0, nlo, nhi, &ov)) {
                UnlockFileEx(h, 0, nlo, nhi, &ov);
                lk->l_type = F_UNLCK;
            }
            return 0;
        }
        if (F_SETLK == cmd) {
            flags |= LOCKFILE_FAIL_IMMEDIATELY;
        }
        /* F_SETLKW (no FAIL_IMMEDIATELY) blocks in LockFileEx until the
         * range becomes available, which is the POSIX semantics. */
        if (!LockFileEx(h, flags, 0, nlo, nhi, &ov)) {
            DWORD gle = GetLastError();
            errno = (ERROR_LOCK_VIOLATION == gle || ERROR_IO_PENDING == gle)
                        ? EACCES
                        : EIO;
            return -1;
        }
        return 0;
    }
    default:
        errno = EINVAL;
        return -1;
    }
}

OPAL_WIN32_DECLSPEC int opal_win32_ioctl(int fd, unsigned long request, ...)
{
    va_list ap;
    va_start(ap, request);

    switch (request) {
    case FIONBIO:
    case FIONREAD:
    case SIOCATMARK: {
        u_long *argp = va_arg(ap, u_long *);
        int rc;
        if (!opal_win32_is_socket(fd)) {
            if (FIONREAD == request) {
                /* CRT fd: bytes available = _eof()/_filelengthi64 */
                DWORD sz = 0;
                *argp = 0;
                errno = ENOTTY;
                va_end(ap);
                return -1;
            }
            errno = ENOTTY;
            va_end(ap);
            return -1;
        }
        rc = ioctlsocket((SOCKET) fd, (long) request, argp);
        if (0 != rc) {
            errno = opal_win32_socket_errno();
            va_end(ap);
            return -1;
        }
        va_end(ap);
        return 0;
    }
    case TIOCGWINSZ: {
        struct winsize *ws = va_arg(ap, struct winsize *);
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
        if (GetConsoleScreenBufferInfo(h, &csbi)) {
            ws->ws_col = (unsigned short) (csbi.srWindow.Right - csbi.srWindow.Left + 1);
            ws->ws_row = (unsigned short) (csbi.srWindow.Bottom - csbi.srWindow.Top + 1);
            ws->ws_xpixel = 0;
            ws->ws_ypixel = 0;
            va_end(ap);
            return 0;
        }
        ws->ws_col = 80;
        ws->ws_row = 24;
        va_end(ap);
        return 0;
    }
    default:
        break;
    }
    va_end(ap);
    errno = ENOTTY;
    return -1;
}

OPAL_WIN32_DECLSPEC int opal_win32_confstr_stub(int name, char *buf, size_t len)
{
    (void) name;
    (void) buf;
    (void) len;
    return 0;
}

OPAL_WIN32_DECLSPEC int opal_win32_getsubopt_stub(char **optionp, char *const *tokens, char **valuep)
{
    (void) optionp;
    (void) tokens;
    (void) valuep;
    errno = ENOSYS;
    return -1;
}

/* ================================================================== */
/* mkstemp / mkdtemp                                                  */
/* ================================================================== */

static const char mkstemp_alpha[] =
    "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

static int mkstemp_try(char *tmpl, int suffixlen)
{
    /* replace 6 X's with random chars; XXXX.. at end minus suffix */
    size_t len = strlen(tmpl);
    int i;
    for (i = 0; i < 6; i++) {
        size_t pos = len - (size_t) suffixlen - 6 + (size_t) i;
        tmpl[pos] = mkstemp_alpha[rand() % 62];
    }
    return 0;
}

OPAL_WIN32_DECLSPEC int opal_win32_mkstemp(char *template_path)
{
    int i;
    for (i = 0; i < 100; i++) {
        int fd;
        mkstemp_try(template_path, 0);
        fd = _open(template_path, _O_RDWR | _O_CREAT | _O_EXCL | _O_BINARY,
                   _S_IREAD | _S_IWRITE);
        if (fd >= 0) {
            return fd;
        }
        if (EEXIST != errno) {
            return -1;
        }
    }
    errno = EEXIST;
    return -1;
}

OPAL_WIN32_DECLSPEC int opal_win32_mkstemps(char *template_path, int suffixlen)
{
    int i;
    for (i = 0; i < 100; i++) {
        int fd;
        mkstemp_try(template_path, suffixlen);
        fd = _open(template_path, _O_RDWR | _O_CREAT | _O_EXCL | _O_BINARY,
                   _S_IREAD | _S_IWRITE);
        if (fd >= 0) {
            return fd;
        }
        if (EEXIST != errno) {
            return -1;
        }
    }
    errno = EEXIST;
    return -1;
}

OPAL_WIN32_DECLSPEC char *opal_win32_mkdtemp(char *template_path)
{
    int i;
    for (i = 0; i < 100; i++) {
        mkstemp_try(template_path, 0);
        if (0 == _mkdir(template_path)) {
            return template_path;
        }
        if (EEXIST != errno) {
            return NULL;
        }
    }
    errno = EEXIST;
    return NULL;
}

/* ================================================================== */
/* environment                                                        */
/* ================================================================== */

OPAL_WIN32_DECLSPEC int setenv(const char *name, const char *value, int overwrite)
{
    if (NULL == name || '\0' == *name || NULL != strchr(name, '=')) {
        errno = EINVAL;
        return -1;
    }
    if (!overwrite && NULL != getenv(name)) {
        return 0;
    }
    return _putenv_s(name, value ? value : "");
}

OPAL_WIN32_DECLSPEC int unsetenv(const char *name)
{
    if (NULL == name || '\0' == *name || NULL != strchr(name, '=')) {
        errno = EINVAL;
        return -1;
    }
    _putenv_s(name, "");
    return 0;
}

/* ================================================================== */
/* string extras                                                      */
/* ================================================================== */

OPAL_WIN32_DECLSPEC int opal_win32_strcasecmp(const char *a, const char *b)
{
    return _stricmp(a, b);
}

OPAL_WIN32_DECLSPEC int strerror_r(int errnum, char *buf, size_t buflen)
{
    if (0 == strerror_s(buf, buflen, errnum)) {
        return 0;
    }
    return EINVAL;
}

char *strtok_r(char *str, const char *delim, char **saveptr)
{
    return strtok_s(str, delim, saveptr);
}

char *strndup(const char *s, size_t n)
{
    size_t len = strnlen(s, n);
    char *r = malloc(len + 1);
    if (NULL == r) {
        return NULL;
    }
    memcpy(r, s, len);
    r[len] = '\0';
    return r;
}

char *strsep(char **stringp, const char *delim)
{
    char *start;
    char *p;
    if (NULL == stringp || NULL == *stringp) {
        return NULL;
    }
    start = *stringp;
    p = strpbrk(start, delim);
    if (p) {
        *p = '\0';
        *stringp = p + 1;
    } else {
        *stringp = NULL;
    }
    return start;
}

OPAL_WIN32_DECLSPEC size_t strlcpy(char *dst, const char *src, size_t siz)
{
    size_t srclen = strlen(src);
    if (siz > 0) {
        size_t copy = (srclen < siz - 1) ? srclen : siz - 1;
        memcpy(dst, src, copy);
        dst[copy] = '\0';
    }
    return srclen;
}

OPAL_WIN32_DECLSPEC size_t strlcat(char *dst, const char *src, size_t siz)
{
    size_t dlen = strnlen(dst, siz);
    size_t slen = strlen(src);
    if (dlen == siz) {
        return siz + slen;
    }
    if (slen < siz - dlen) {
        memcpy(dst + dlen, src, slen + 1);
    } else {
        memcpy(dst + dlen, src, siz - dlen - 1);
        dst[siz - 1] = '\0';
    }
    return dlen + slen;
}

void *memrchr(const void *s, int c, size_t n)
{
    const unsigned char *p = (const unsigned char *) s;
    while (n-- > 0) {
        if (p[n] == (unsigned char) c) {
            return (void *) (p + n);
        }
    }
    return NULL;
}

void *mempcpy(void *dest, const void *src, size_t n)
{
    return (char *) memcpy(dest, src, n) + n;
}

char *stpcpy(char *dest, const char *src)
{
    size_t len = strlen(src);
    memcpy(dest, src, len + 1);
    return dest + len;
}

/* ================================================================== */
/* stdlib extras                                                      */
/* ================================================================== */

OPAL_WIN32_DECLSPEC int posix_memalign(void **memptr, size_t alignment, size_t size)
{
    void *p;
    if (0 == alignment || (alignment & (alignment - 1)) != 0
        || alignment < sizeof(void *)) {
        return EINVAL;
    }
    p = _aligned_malloc(size, alignment);
    if (NULL == p) {
        return ENOMEM;
    }
    *memptr = p;
    return 0;
}

char *realpath(const char *path, char *resolved)
{
    char buf[MAX_PATH];
    DWORD n = GetFullPathNameA(path, MAX_PATH, buf, NULL);
    if (0 == n || n >= MAX_PATH) {
        errno = ENOENT;
        return NULL;
    }
    if (NULL == resolved) {
        resolved = malloc(MAX_PATH);
        if (NULL == resolved) {
            errno = ENOMEM;
            return NULL;
        }
    }
    strcpy(resolved, buf);
    return resolved;
}

OPAL_WIN32_DECLSPEC int getloadavg(double loadavg[], int nelem)
{
    int i;
    for (i = 0; i < nelem; i++) {
        loadavg[i] = 0.0;
    }
    return nelem;
}

OPAL_WIN32_DECLSPEC long random(void)
{
    return ((long) rand() << 16) ^ rand();
}

OPAL_WIN32_DECLSPEC void srandom(unsigned int seed)
{
    srand(seed);
}

/* ================================================================== */
/* stdio extras                                                       */
/* ================================================================== */

OPAL_WIN32_DECLSPEC ssize_t getline(char **lineptr, size_t *n, FILE *stream)
{
    return getdelim(lineptr, n, '\n', stream);
}

OPAL_WIN32_DECLSPEC ssize_t getdelim(char **lineptr, size_t *n, int delim, FILE *stream)
{
    size_t pos = 0;
    int c;
    if (NULL == *lineptr || 0 == *n) {
        *n = 128;
        *lineptr = malloc(*n);
        if (NULL == *lineptr) {
            return -1;
        }
    }
    while (EOF != (c = fgetc(stream))) {
        if (pos + 1 >= *n) {
            size_t nn = *n * 2;
            char *nb = realloc(*lineptr, nn);
            if (NULL == nb) {
                return -1;
            }
            *lineptr = nb;
            *n = nn;
        }
        (*lineptr)[pos++] = (char) c;
        if (c == delim) {
            break;
        }
    }
    if (0 == pos) {
        return -1;
    }
    (*lineptr)[pos] = '\0';
    return (ssize_t) pos;
}

OPAL_WIN32_DECLSPEC int dprintf(int fd, const char *format, ...)
{
    va_list ap;
    char buf[8192];
    int n;
    va_start(ap, format);
    n = _vsnprintf(buf, sizeof(buf), format, ap);
    va_end(ap);
    if (n < 0) {
        return -1;
    }
    return _write(fd, buf, n);
}

OPAL_WIN32_DECLSPEC int vasprintf(char **strp, const char *fmt, va_list ap)
{
    int n = _vscprintf(fmt, ap);
    if (n < 0) {
        return -1;
    }
    *strp = malloc((size_t) n + 1);
    if (NULL == *strp) {
        return -1;
    }
    return _vsnprintf(*strp, (size_t) n + 1, fmt, ap);
}

OPAL_WIN32_DECLSPEC int asprintf(char **strp, const char *fmt, ...)
{
    va_list ap;
    int rc;
    va_start(ap, fmt);
    rc = vasprintf(strp, fmt, ap);
    va_end(ap);
    return rc;
}

OPAL_WIN32_DECLSPEC int vdprintf(int fd, const char *format, va_list ap)
{
    char buf[8192];
    int n = _vsnprintf(buf, sizeof(buf), format, ap);
    if (n < 0) {
        return -1;
    }
    return _write(fd, buf, n);
}

FILE *popen(const char *command, const char *mode)
{
    return _popen(command, mode);
}

OPAL_WIN32_DECLSPEC int pclose(FILE *stream)
{
    return _pclose(stream);
}

OPAL_WIN32_DECLSPEC int getw(FILE *stream)
{
    return fgetc(stream);
}

OPAL_WIN32_DECLSPEC int putw(int w, FILE *stream)
{
    return fputc(w, stream);
}

OPAL_WIN32_DECLSPEC void setlinebuf(FILE *stream)
{
    setvbuf(stream, NULL, _IOLBF, 0);
}

OPAL_WIN32_DECLSPEC int fchmod(int fd, mode_t mode)
{
    (void) fd;
    (void) mode;
    return 0;
}

OPAL_WIN32_DECLSPEC int fchown(int fd, uid_t owner, gid_t group)
{
    (void) fd;
    (void) owner;
    (void) group;
    return 0;
}

OPAL_WIN32_DECLSPEC int lockf(int fd, int cmd, off_t len)
{
    (void) fd;
    (void) cmd;
    (void) len;
    return 0;
}

OPAL_WIN32_DECLSPEC int mkdir_p(const char *path, mode_t mode)
{
    (void) mode;
    return _mkdir(path);
}

/* ================================================================== */
/* flock() over LockFileEx                                            */
/* ================================================================== */

struct flock_ent {
    int fd;
    struct flock_ent *next;
};
static struct flock_ent *flock_list = NULL;

OPAL_WIN32_DECLSPEC int opal_win32_flock(int fd, int operation)
{
    HANDLE h = (HANDLE) _get_osfhandle(fd);
    OVERLAPPED ov;
    DWORD flags = 0;
    BOOL rc;

    if (INVALID_HANDLE_VALUE == h) {
        errno = EBADF;
        return -1;
    }
    memset(&ov, 0, sizeof(ov));
    if (operation & LOCK_UN) {
        rc = UnlockFileEx(h, 0, 0xFFFFFFFF, 0xFFFFFFFF, &ov);
        return rc ? 0 : -1;
    }
    if (operation & LOCK_EX) {
        flags |= LOCKFILE_EXCLUSIVE_LOCK;
    }
    if (operation & LOCK_NB) {
        flags |= LOCKFILE_FAIL_IMMEDIATELY;
    }
    rc = LockFileEx(h, flags, 0, 0xFFFFFFFF, 0xFFFFFFFF, &ov);
    if (!rc) {
        errno = (GetLastError() == ERROR_LOCK_VIOLATION
                     || GetLastError() == ERROR_IO_PENDING)
                    ? EWOULDBLOCK
                    : EIO;
        return -1;
    }
    return 0;
}

/* ================================================================== */
/* uname / sysinfo                                                    */
/* ================================================================== */

OPAL_WIN32_DECLSPEC int opal_win32_uname(struct utsname *buf)
{
    SYSTEM_INFO si;
    OSVERSIONINFOEXA vi;
    DWORD sz = sizeof(buf->nodename);

    memset(buf, 0, sizeof(*buf));
    strcpy(buf->sysname, "Windows");
    GetComputerNameA(buf->nodename, &sz);
    RtlZeroMemory(&vi, sizeof(vi));
    vi.dwOSVersionInfoSize = sizeof(vi);
#pragma warning(push)
#pragma warning(disable : 4996)
    GetVersionExA((OSVERSIONINFOA *) &vi);
#pragma warning(pop)
    _snprintf(buf->release, sizeof(buf->release), "%lu.%lu", vi.dwMajorVersion,
              vi.dwMinorVersion);
    _snprintf(buf->version, sizeof(buf->version), "Build %lu", vi.dwBuildNumber);
    GetSystemInfo(&si);
    switch (si.wProcessorArchitecture) {
    case PROCESSOR_ARCHITECTURE_AMD64:
        strcpy(buf->machine, "x86_64");
        break;
    case PROCESSOR_ARCHITECTURE_ARM64:
        strcpy(buf->machine, "arm64");
        break;
    case PROCESSOR_ARCHITECTURE_INTEL:
        strcpy(buf->machine, "x86");
        break;
    default:
        strcpy(buf->machine, "unknown");
        break;
    }
    buf->domainname[0] = '\0';
    return 0;
}

/* ================================================================== */
/* statvfs                                                            */
/* ================================================================== */

OPAL_WIN32_DECLSPEC int statvfs(const char *path, struct statvfs *buf)
{
    ULARGE_INTEGER avail, total, freeb;
    char root[MAX_PATH];
    DWORD spc, bps, nfc, tnc;
    const char *p = path;
    DWORD attrs;

    memset(buf, 0, sizeof(*buf));
    memset(root, 0, sizeof(root));
    if (NULL == path) {
        errno = EINVAL;
        return -1;
    }
    /* get drive root */
    if (strlen(path) >= 2 && path[1] == ':') {
        root[0] = path[0];
        root[1] = ':';
        root[2] = '\\';
    } else {
        strcpy(root, "C:\\");
    }
    if (!GetDiskFreeSpaceExA(root, &avail, &total, &freeb)) {
        errno = ENOENT;
        return -1;
    }
    if (!GetDiskFreeSpaceA(root, &spc, &bps, &nfc, &tnc)) {
        spc = 8;
        bps = 512;
        nfc = 0;
        tnc = 0;
    }
    buf->f_bsize = bps * spc;
    buf->f_frsize = bps;
    buf->f_blocks = (fsblkcnt_t) (total.QuadPart / (bps > 0 ? bps : 512));
    buf->f_bfree = (fsblkcnt_t) (freeb.QuadPart / (bps > 0 ? bps : 512));
    buf->f_bavail = (fsblkcnt_t) (avail.QuadPart / (bps > 0 ? bps : 512));
    buf->f_files = 0;
    buf->f_ffree = 0;
    buf->f_favail = 0;
    buf->f_fsid = 0;
    attrs = GetFileAttributesA(root);
    buf->f_flag = 0;
    buf->f_namemax = 255;
    (void) p;
    (void) attrs;
    return 0;
}

OPAL_WIN32_DECLSPEC int fstatvfs(int fd, struct statvfs *buf)
{
    (void) fd;
    return statvfs("C:\\", buf);
}

/* ================================================================== */
/* times()                                                            */
/* ================================================================== */

OPAL_WIN32_DECLSPEC clock_t opal_win32_times(struct tms *buf)
{
    FILETIME c, e, k, u;
    ULONGLONG kt, ut;
    if (0 == GetProcessTimes(GetCurrentProcess(), &c, &e, &k, &u)) {
        return (clock_t) -1;
    }
    kt = (((ULONGLONG) k.dwHighDateTime << 32) | k.dwLowDateTime) / 10000ULL;
    ut = (((ULONGLONG) u.dwHighDateTime << 32) | u.dwLowDateTime) / 10000ULL;
    if (buf) {
        buf->tms_utime = (clock_t) ut;
        buf->tms_stime = (clock_t) kt;
        buf->tms_cutime = 0;
        buf->tms_cstime = 0;
    }
    return (clock_t) ut;
}

/* ================================================================== */
/* stat() variants                                                    */
/* ================================================================== */

OPAL_WIN32_DECLSPEC int lstat(const char *path, struct stat *buf)
{
    return _stat(path, buf);
}

OPAL_WIN32_DECLSPEC int fstatat(int dirfd, const char *pathname, struct stat *buf, int flags)
{
    char full[MAX_PATH];
    if (0 != opal_win32_resolve_at(dirfd, pathname, full, sizeof(full))) {
        return -1;
    }
    if (flags & AT_SYMLINK_NOFOLLOW) {
        return lstat(full, buf);
    }
    return _stat(full, buf);
}

OPAL_WIN32_DECLSPEC int mkdirat(int dirfd, const char *pathname, mode_t mode)
{
    char full[MAX_PATH];
    (void) mode;
    if (0 != opal_win32_resolve_at(dirfd, pathname, full, sizeof(full))) {
        return -1;
    }
    return _mkdir(full);
}

OPAL_WIN32_DECLSPEC int mkfifo(const char *path, mode_t mode)
{
    (void) path;
    (void) mode;
    errno = ENOSYS;
    return -1;
}

OPAL_WIN32_DECLSPEC int mknod(const char *path, mode_t mode, dev_t dev)
{
    (void) path;
    (void) mode;
    (void) dev;
    errno = ENOSYS;
    return -1;
}

/* ================================================================== */
/* PTY stubs (no ptys on Windows; PRRTE iof pty backend is disabled)  */
/* ================================================================== */

OPAL_WIN32_DECLSPEC int posix_openpt(int flags)
{
    (void) flags;
    errno = ENOSYS;
    return -1;
}

OPAL_WIN32_DECLSPEC int grantpt(int fd)
{
    (void) fd;
    return 0;
}

OPAL_WIN32_DECLSPEC int unlockpt(int fd)
{
    (void) fd;
    return 0;
}

char *ptsname(int fd)
{
    static char namebuf[64];
    (void) fd;
    strcpy(namebuf, "\\\\?\\pty");
    return namebuf;
}

OPAL_WIN32_DECLSPEC int ptsname_r(int fd, char *buf, size_t buflen)
{
    const char *n = ptsname(fd);
    (void) fd;
    if (NULL == n || strlen(n) + 1 > buflen) {
        return ERANGE;
    }
    strcpy(buf, n);
    return 0;
}

char *fgetln(FILE *stream, size_t *len)
{
    static char buf[4096];
    if (NULL == fgets(buf, (int) sizeof(buf), stream)) {
        return NULL;
    }
    if (len) {
        *len = strlen(buf);
    }
    return buf;
}

void *valloc(size_t size)
{
    return _aligned_malloc(size, opal_win32_getpagesize());
}

void *memalign(size_t alignment, size_t size)
{
    return _aligned_malloc(size, alignment);
}

/* ================================================================== */
/* basename / dirname (libgen.h)                                       */
/* ================================================================== */

OPAL_WIN32_DECLSPEC char *opal_win32_basename(char *path)
{
    static char buf[MAX_PATH];
    char *p, *base;

    if (NULL == path) {
        strcpy(buf, ".");
        return buf;
    }
    /* strip trailing separators */
    p = path + strlen(path);
    while (p > path && ('/' == p[-1] || '\\' == p[-1])) {
        *--p = '\0';
    }
    base = p;
    while (base > path && '/' != base[-1] && '\\' != base[-1]) {
        --base;
    }
    if ('\0' == *base) {
        strcpy(buf, "/");
        return buf;
    }
    strcpy(buf, base);
    return buf;
}

OPAL_WIN32_DECLSPEC char *opal_win32_dirname(char *path)
{
    static char buf[MAX_PATH];
    char *p;

    if (NULL == path) {
        strcpy(buf, ".");
        return buf;
    }
    strcpy(buf, path);
    /* strip trailing separators */
    p = buf + strlen(buf);
    while (p > buf && ('/' == p[-1] || '\\' == p[-1])) {
        *--p = '\0';
    }
    /* find last separator */
    while (p > buf && '/' != p[-1] && '\\' != p[-1]) {
        --p;
    }
    while (p > buf && ('/' == p[-1] || '\\' == p[-1])) {
        *--p = '\0';
    }
    if ('\0' == *buf) {
        strcpy(buf, (p == buf) ? "/" : ".");
    }
    return buf;
}

#endif /* _WIN32 */
