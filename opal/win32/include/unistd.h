/*
 * Copyright (c) 2026      Amazon.com, Inc. or its affiliates.
 *                         All Rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 *
 * POSIX <unistd.h> replacement for the native Windows (MSVC) build of
 * Open MPI.  MSVC has no such header; this provides the declarations
 * and macro shims that the code base expects.  Function bodies live in
 * opal/win32/opal_win32_compat.c.
 */

#ifndef OPAL_WIN32_UNISTD_H
#define OPAL_WIN32_UNISTD_H

#include "opal_win32_common.h"
#include <sys/types.h>
#include <time.h>

struct iovec;

#ifdef __cplusplus
extern "C" {
#endif

/* File access bits (access()/_access()) */
#ifndef F_OK
#    define F_OK 0
#endif
#ifndef X_OK
#    define X_OK 1 /* not really meaningful on Windows */
#endif
#ifndef W_OK
#    define W_OK 2
#endif
#ifndef R_OK
#    define R_OK 4
#endif

/* Map the handful of POSIX fd functions onto the MSVCRT equivalents.
 * CRT "fds" are indices into a per-process handle table -- they are
 * NOT interchangeable with Win32 HANDLEs or SOCKETs.  Code paths that
 * need true HANDLE semantics must branch on _WIN32. */
/* close() is disambiguated at runtime: CRT fds go to _close, sockets
 * to closesocket.  Do NOT redefine it to _close. */
#define open        opal_win32_open
#define close       opal_win32_close
/* read/write are disambiguated like close: fds registered as sockets
 * route to recv/send so that socketpair() "pipes" remain usable through
 * the POSIX API spelling. */
#define read        opal_win32_read
#define write       opal_win32_write
#define lseek       _lseeki64
#define lseek64     _lseeki64
#define tell        _telli64
#define dup         _dup
#define dup2        _dup2
#define isatty      _isatty
#define access      _access
#define unlink      _unlink
/* fileno_unlocked is the glibc lock-free fileno; _fileno is correct (the
 * lock is only a perf detail) */
#define fileno_unlocked _fileno
/* per-process fd-table bound; CRT caps at _getmaxstdio (8192 default) */
#define getdtablesize _getmaxstdio

/* standard stream fd numbers (POSIX) */
#ifndef STDIN_FILENO
#    define STDIN_FILENO  0
#    define STDOUT_FILENO 1
#    define STDERR_FILENO 2
#endif
#define rmdir       _rmdir
#define chdir       _chdir
#define getcwd      _getcwd
/* chmod/umask are provided by sys/stat.h */
#define fsync       _commit
#define fdatasync   _commit
#define getpid      _getpid
#define getppid()   0
#define pipe(fds)   _pipe((fds), 4096, _O_BINARY | _O_NOINHERIT)
#define pipe2(fds, fl) opal_win32_pipe2((fds), (fl))

#ifndef _GETPID_DECLARED
#    define _GETPID_DECLARED
#endif

/* exec* family: implemented with CreateProcess where possible */
#define execl   opal_win32_execl_stub
#define execle  opal_win32_execle_stub
#define execlp  opal_win32_execlp_stub
#define execv   opal_win32_execv
#define execve  opal_win32_execve
#define execvp  opal_win32_execvp
#define execlpe opal_win32_execlpe_stub
#define fork    opal_win32_fork_stub
#define vfork   opal_win32_fork_stub

/* I/O multiplexing / misc */
#define gethostname     opal_win32_gethostname
#define getpagesize     opal_win32_getpagesize
#define sysconf         opal_win32_sysconf
#define sleep           opal_win32_sleep
#define usleep          opal_win32_usleep
/* nanosleep is declared in <time.h> (POSIX); the impl uses the literal
 * name there, so keep it literal here too -- no CRT collision exists */
#define truncate        opal_win32_truncate
#define ftruncate       opal_win32_ftruncate
#define getlogin        opal_win32_getlogin
#define getlogin_r      opal_win32_getlogin_r
/* Windows has no POSIX uid and no "uid 0 == superuser" rule; returning 0
 * would make every caller believe it is running as root (PRRTE/PMIx
 * refuse to run as root by default).  Use a fixed non-zero uid. */
#define getuid()        ((uid_t) 1000)
#define geteuid()       ((uid_t) 1000)
#define getgid()        ((gid_t) 0)
#define getegid()       ((gid_t) 0)
#define setuid(x)       (0)
#define setgid(x)       (0)
#define setegid(x)      (0)
#define seteuid(x)      (0)
#define getgroups(n, g) (0)
#define link(o, n)      opal_win32_link((o), (n))
#define symlink(o, n)   opal_win32_symlink((o), (n))
#define readlink(p, b, s) opal_win32_readlink((p), (b), (s))
/* lstat is declared in sys/stat.h */
#define readlinkat(a, p, b, s) opal_win32_readlink((p), (b), (s))
#define nice(x)         (0)
#define sync()          do { } while (0)
#define fsync_range(f, o, n) _commit(f)
#define alarm(x)        (0)
#define pause()         Sleep(INFINITE)
#define chroot(p)       (-1)
#define setpgid(p, g)   (0)
#define setsid()        (0)
#define getpgrp()       (0)
/* no POSIX process groups: callers that test "if (-1 != getpgid(pd))
 * target the group lead" fall back to signalling the pid directly */
#define getpgid(p)      (-1)
#define tcgetpgrp(f)    (-1)
#define tcsetpgrp(f, g) (-1)
#define sethostname(n, l) (0)
#define getdomainname(n, l) (0)
#define fcntl           opal_win32_fcntl
#define ioctl           opal_win32_ioctl
#define confstr         opal_win32_confstr_stub
#define getopt          opal_getopt
#define optarg          opal_optarg
#define optind          opal_optind
#define opterr          opal_opterr
#define optopt          opal_optopt
#define getsubopt       opal_win32_getsubopt_stub
#define mkstemp         opal_win32_mkstemp
#define mkstemps        opal_win32_mkstemps
#define mkdtemp         opal_win32_mkdtemp
#define pread           opal_win32_pread
#define pwrite          opal_win32_pwrite
#define readv           opal_win32_readv
#define writev          opal_win32_writev
#define _SC_ARG_MAX            1
#define _SC_CHILD_MAX          2
#define _SC_CLK_TCK            3
#define _SC_NGROUPS_MAX        4
#define _SC_OPEN_MAX           5
#define _SC_JOB_CONTROL        6
#define _SC_SAVED_IDS          7
#define _SC_VERSION            8
#define _SC_PAGESIZE           9
#define _SC_PAGE_SIZE          _SC_PAGESIZE
#define _SC_NPROCESSORS_CONF   10
#define _SC_NPROCESSORS_ONLN   11
#define _SC_PHYS_PAGES         12
#define _SC_AVPHYS_PAGES       13
#define _SC_HOST_NAME_MAX      14
#define _SC_LOGIN_NAME_MAX     15
#define _SC_LINE_MAX           16
#define _SC_BC_BASE_MAX        17
#define _SC_STREAM_MAX         18
#define _SC_TZNAME_MAX         19
#define _SC_IOV_MAX            20
#define _SC_SYMLOOP_MAX        21
#define _SC_GETPW_R_SIZE_MAX   22
#define _SC_GETGR_R_SIZE_MAX   23
#define _SC_MAPPED_FILES       24
#define _SC_MEMLOCK            25
#define _SC_MEMLOCK_RANGE      26
#define _SC_MEMORY_PROTECTION  27
#define _SC_MESSAGE_PASSING    28
#define _SC_PRIORITIZED_IO     29
#define _SC_PRIORITY_SCHEDULING 30
#define _SC_REALTIME_SIGNALS   31
#define _SC_SEMAPHORES         32
#define _SC_SHARED_MEMORY_OBJECTS 33
#define _SC_SYNCHRONIZED_IO    34
#define _SC_TIMERS             35
#define _SC_AIO_LISTIO_MAX     36
#define _SC_AIO_MAX            37
#define _SC_AIO_PRIO_DELTA_MAX 38
#define _SC_DELAYTIMER_MAX     39
#define _SC_MQ_OPEN_MAX        40
#define _SC_RTSIG_MAX          41
#define _SC_SIGQUEUE_MAX       42
#define _SC_TIMER_MAX          43
#define _SC_THREAD_MAX         44
#define _SC_XOPEN_VERSION      45

/* Environment */
#define environ _environ
#define clearenv() (0)

/* Declarations for the Windows implementations (opal_win32_compat.c) */
OPAL_WIN32_DECLSPEC int opal_win32_open(const char *pathname, int flags, ...);
OPAL_WIN32_DECLSPEC int opal_win32_close(int fd);
OPAL_WIN32_DECLSPEC ssize_t opal_win32_read(int fd, void *buf, size_t count);
OPAL_WIN32_DECLSPEC ssize_t opal_win32_write(int fd, const void *buf, size_t count);
OPAL_WIN32_DECLSPEC int opal_win32_pipe2(int fds[2], int flags);
OPAL_WIN32_DECLSPEC int opal_win32_execv(const char *path, char *const argv[]);
OPAL_WIN32_DECLSPEC int opal_win32_execve(const char *path, char *const argv[], char *const envp[]);
OPAL_WIN32_DECLSPEC int opal_win32_execvp(const char *file, char *const argv[]);
OPAL_WIN32_DECLSPEC int opal_win32_execl_stub(const char *path, const char *arg0, ...);
OPAL_WIN32_DECLSPEC int opal_win32_execle_stub(const char *path, const char *arg0, ...);
OPAL_WIN32_DECLSPEC int opal_win32_execlp_stub(const char *file, const char *arg0, ...);
OPAL_WIN32_DECLSPEC int opal_win32_execlpe_stub(const char *file, const char *arg0, ...);
OPAL_WIN32_DECLSPEC pid_t opal_win32_fork_stub(void);
OPAL_WIN32_DECLSPEC int opal_win32_gethostname(char *name, size_t len);
OPAL_WIN32_DECLSPEC int opal_win32_getpagesize(void);
OPAL_WIN32_DECLSPEC long opal_win32_sysconf(int name);
OPAL_WIN32_DECLSPEC unsigned int opal_win32_sleep(unsigned int seconds);
OPAL_WIN32_DECLSPEC int opal_win32_usleep(useconds_t usec);
OPAL_WIN32_DECLSPEC int nanosleep(const struct timespec *req, struct timespec *rem);
OPAL_WIN32_DECLSPEC int unlinkat(int dirfd, const char *pathname, int flags);
OPAL_WIN32_DECLSPEC int opal_win32_truncate(const char *path, off_t length);
OPAL_WIN32_DECLSPEC int opal_win32_ftruncate(int fd, off_t length);
OPAL_WIN32_DECLSPEC char *opal_win32_getlogin(void);
OPAL_WIN32_DECLSPEC int opal_win32_getlogin_r(char *buf, size_t bufsize);
OPAL_WIN32_DECLSPEC int opal_win32_link(const char *oldpath, const char *newpath);
OPAL_WIN32_DECLSPEC int opal_win32_symlink(const char *oldpath, const char *newpath);
OPAL_WIN32_DECLSPEC ssize_t opal_win32_readlink(const char *path, char *buf, size_t bufsize);
OPAL_WIN32_DECLSPEC int opal_win32_fcntl(int fd, int cmd, ...);
OPAL_WIN32_DECLSPEC int opal_win32_ioctl(int fd, unsigned long request, ...);
OPAL_WIN32_DECLSPEC int opal_win32_confstr_stub(int name, char *buf, size_t len);
OPAL_WIN32_DECLSPEC int opal_win32_getsubopt_stub(char **optionp, char *const *tokens, char **valuep);
OPAL_WIN32_DECLSPEC int opal_win32_mkstemp(char *template_path);
OPAL_WIN32_DECLSPEC int opal_win32_mkstemps(char *template_path, int suffixlen);
OPAL_WIN32_DECLSPEC char *opal_win32_mkdtemp(char *template_path);
OPAL_WIN32_DECLSPEC ssize_t opal_win32_pread(int fd, void *buf, size_t count, off_t offset);
OPAL_WIN32_DECLSPEC ssize_t opal_win32_pwrite(int fd, const void *buf, size_t count, off_t offset);
OPAL_WIN32_DECLSPEC ssize_t opal_win32_readv(int fd, const struct iovec *iov, int iovcnt);
OPAL_WIN32_DECLSPEC ssize_t opal_win32_writev(int fd, const struct iovec *iov, int iovcnt);

/* literal POSIX names implemented in the compat TU (no CRT collision) */
OPAL_WIN32_DECLSPEC int fchown(int fd, uid_t owner, gid_t group);
OPAL_WIN32_DECLSPEC int lockf(int fd, int cmd, off_t len);
OPAL_WIN32_DECLSPEC int daemon(int nochdir, int noclose);
OPAL_WIN32_DECLSPEC int fchmod(int fd, mode_t mode);
OPAL_WIN32_DECLSPEC int mkdir_p(const char *path, mode_t mode);
OPAL_WIN32_DECLSPEC int setenv(const char *name, const char *value, int overwrite);
OPAL_WIN32_DECLSPEC int unsetenv(const char *name);
int          chown(const char *path, uid_t owner, gid_t group);
int          lchown(const char *path, uid_t owner, gid_t group);

/* getopt */
OPAL_WIN32_DECLSPEC extern char *opal_optarg;
OPAL_WIN32_DECLSPEC extern int opal_optind, opal_opterr, opal_optopt;
OPAL_WIN32_DECLSPEC int opal_getopt(int argc, char *const argv[], const char *optstring);

#ifdef __cplusplus
}
#endif

#endif /* OPAL_WIN32_UNISTD_H */
