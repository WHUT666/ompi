/* -*- Mode: C; c-basic-offset:4 ; indent-tabs-mode:nil -*- */
/*
 * pmix_config.h template for the native Windows (MSVC/CMake) build.
 *
 * The autotools build generates this header with autoheader; for the
 * Windows port the feature answers are fixed by the opal/win32
 * compatibility layer (shared with open-pal), so this file is
 * configured with a small set of @VAR@ substitutions and everything
 * else spelled out literally.
 */

#ifndef PMIX_CONFIG_H
#define PMIX_CONFIG_H

#include "src/include/pmix_config_top.h"

/* -- version ------------------------------------------------------ */
#define PMIX_VERSION "@pmixversion@"
#define PMIX_MAJOR_VERSION @pmixmajor@
#define PMIX_MINOR_VERSION @pmixminor@
#define PMIX_RELEASE_VERSION @pmixrelease@
#define PMIX_GREEK_VERSION "@pmixgreek@"
#define PMIX_REPO_REV "@pmixreporev@"
#define PMIX_RELEASE_DATE "@pmixreleasedate@"
#define PMIX_PACKAGE_STRING "PMIX @pmixversion@"
#define PMIX_PACKAGE_VERSION PMIX_VERSION
#define PMIX_IDENT_STRING PMIX_PACKAGE_STRING

#define PACKAGE_NAME "pmix"
#define PACKAGE_STRING PMIX_PACKAGE_STRING
#define PACKAGE_TARNAME "pmix"
#define PACKAGE_VERSION "@pmixversion@"
#define PACKAGE_BUGREPORT "https://github.com/openpmix/openpmix/issues"
#define PACKAGE_URL "https://pmix.org/"

/* -- install directories ------------------------------------------ */
/* NB: PMIX_PREFIX and PMIX_LOG_SYSLOG are NOT config macros -- they are
 * PMIx attribute-key strings defined by pmix_common.h.  Autotools does
 * leak them into config.h, but no source reads them that way, and
 * redefining them here only generates C4005 noise. */
#define PMIX_BINDIR "@PMIX_BINDIR@"
#define PMIX_SBINDIR "@PMIX_SBINDIR@"
#define PMIX_LIBEXECDIR "@PMIX_LIBEXECDIR@"
#define PMIX_DATAROOTDIR "@PMIX_DATAROOTDIR@"
#define PMIX_DATADIR "@PMIX_DATADIR@"
#define PMIX_SYSCONFDIR "@PMIX_SYSCONFDIR@"
#define PMIX_SHAREDSTATEDIR "@PMIX_SHAREDSTATEDIR@"
#define PMIX_LOCALSTATEDIR "@PMIX_LOCALSTATEDIR@"
#define PMIX_LIBDIR "@PMIX_LIBDIR@"
#define PMIX_INCLUDEDIR "@PMIX_INCLUDEDIR@"
#define PMIX_INFODIR "@PMIX_INFODIR@"
#define PMIX_MANDIR "@PMIX_MANDIR@"
#define PMIX_PKGDATADIR "@PMIX_PKGDATADIR@"
#define PMIX_PKGLIBDIR "@PMIX_PKGLIBDIR@"
#define PMIX_PKGINCLUDEDIR "@PMIX_PKGINCLUDEDIR@"

#define PMIX_PARAM_DEFAULT_FILES_PREFIX "@PMIX_BINDIR@/.."
#define PMIX_WANT_HOME_CONFIG_FILES 1
#define PMIX_SET_USEABLE 1

/* -- compiler ----------------------------------------------------- */
#define PMIX_CC "cl"
#define PMIX_HAVE_VISIBILITY 0
#define PMIX_C_HAVE_VISIBILITY 0
#define PMIX_HAVE_ATTRIBUTE_ALIGNED 0
#define PMIX_HAVE_ATTRIBUTE_ALIGNED_FUNCPTR 0
#define PMIX_HAVE_ATTRIBUTE_ALWAYS_INLINE 0
#define PMIX_HAVE_ATTRIBUTE_COLD 0
#define PMIX_HAVE_ATTRIBUTE_CONST 0
#define PMIX_HAVE_ATTRIBUTE_CONSTRUCTOR 0
#define PMIX_HAVE_ATTRIBUTE_DEPRECATED 0
#define PMIX_HAVE_ATTRIBUTE_DEPRECATED_ARGUMENT 0
#define PMIX_HAVE_ATTRIBUTE_DESTRUCTOR 0
#define PMIX_HAVE_ATTRIBUTE_ERROR 0
#define PMIX_HAVE_ATTRIBUTE_EXTENSION 0
#define PMIX_HAVE_ATTRIBUTE_FORMAT 0
#define PMIX_HAVE_ATTRIBUTE_FORMAT_FUNCPTR 0
#define PMIX_HAVE_ATTRIBUTE_HOT 0
#define PMIX_HAVE_ATTRIBUTE_MALLOC 0
#define PMIX_HAVE_ATTRIBUTE_MAY_ALIAS 0
#define PMIX_HAVE_ATTRIBUTE_NO_INSTRUMENT_FUNCTION 0
#define PMIX_HAVE_ATTRIBUTE_NO_SANITIZE 0
#define PMIX_HAVE_ATTRIBUTE_NO_SANITIZE_ADDRESS 0
#define PMIX_HAVE_ATTRIBUTE_NO_SANITIZE_THREAD 0
#define PMIX_HAVE_ATTRIBUTE_NO_SANITIZE_UNDEFINED 0
#define PMIX_HAVE_ATTRIBUTE_NOINLINE 0
#define PMIX_HAVE_ATTRIBUTE_NONNULL 0
#define PMIX_HAVE_ATTRIBUTE_NORETURN 0
#define PMIX_HAVE_ATTRIBUTE_NORETURN_FUNCPTR 0
#define PMIX_HAVE_ATTRIBUTE_OPTNONE 0
#define PMIX_HAVE_ATTRIBUTE_PACKED 0
#define PMIX_HAVE_ATTRIBUTE_PURE 0
#define PMIX_HAVE_ATTRIBUTE_SENTINEL 0
#define PMIX_HAVE_ATTRIBUTE_UNUSED 0
#define PMIX_HAVE_ATTRIBUTE_VISIBILITY 0
#define PMIX_HAVE_ATTRIBUTE_WARN_UNUSED_RESULT 0
#define PMIX_HAVE_ATTRIBUTE_WEAK_ALIAS 0
#define PMIX_HAVE_WEAK_SYMBOLS 0
#define PMIX_HAVE_VA_COPY 1
#define PMIX_HAVE_UNDERSCORE_VA_COPY 1
#define PMIX_PICKY_COMPILERS 0

/* -- type sizes (x86_64 Windows/LLP64) ---------------------------- */
#define SIZEOF_CHAR 1
#define SIZEOF_SHORT 2
#define SIZEOF_WCHAR_T 2
#define SIZEOF_INT 4
#define SIZEOF_LONG 4
#define SIZEOF_LONG_LONG 8
#define SIZEOF_PTRDIFF_T 8
#define SIZEOF_SIZE_T 8
#define SIZEOF_SSIZE_T 8
#define SIZEOF_VOID_P 8
#define SIZEOF_PID_T 4
#define SIZEOF_OFF_T 4
#define SIZEOF_BOOL 1
#define SIZEOF__BOOL 1
#define SIZEOF_UID_T 4
#define SIZEOF_GID_T 4
#define SIZEOF_MODE_T 2
#define SIZEOF_SIGSET_T 8

#define HAVE_INTPTR_T 1
#define HAVE_UINTPTR_T 1
#define HAVE_PTRDIFF_T 1
#define PMIX_PTRDIFF_TYPE ptrdiff_t
#define HAVE_LONG_LONG 1
#define HAVE_SOCKLEN_T 1
#define HAVE_STRUCT_SOCKADDR_IN 1
#define HAVE_STRUCT_SOCKADDR_IN6 1
#define HAVE_STRUCT_SOCKADDR_STORAGE 1
#define HAVE_DECL_AF_INET6 1
#define HAVE_DECL_AF_UNSPEC 1
#define HAVE_DECL_PF_INET6 1
#define HAVE_DECL_PF_UNSPEC 1

/* -- headers (provided by UCRT or the opal/win32 shadow layer) ----- */
#define HAVE_ASSERT_H 1
#define HAVE_CTYPE_H 1
#define HAVE_DIRENT_H 1
#define HAVE_ERRNO_H 1
#define HAVE_FCNTL_H 1
#define HAVE_IFADDRS_H 1
#define HAVE_INTTYPES_H 1
#define HAVE_LIMITS_H 1
#define HAVE_LOCALE_H 1
#define HAVE_MEMORY_H 1
#define HAVE_NETDB_H 1
#define HAVE_NETINET_IN_H 1
#define HAVE_NETINET_TCP_H 1
#define HAVE_ARPA_INET_H 1
#define HAVE_SIGNAL_H 1
#define HAVE_STDBOOL_H 1
#define HAVE_STDATOMIC_H 1
#define HAVE_STDDEF_H 1
#define HAVE_STDINT_H 1
#define HAVE_STDIO_H 1
#define HAVE_STDLIB_H 1
#define HAVE_STRING_H 1
#define HAVE_STRINGS_H 1
#define HAVE_SYS_STAT_H 1
#define HAVE_SYS_TIME_H 1
#define HAVE_SYS_TYPES_H 1
#define HAVE_SYS_UIO_H 1
#define HAVE_SYS_UTIME_H 1
#define HAVE_SYS_WAIT_H 1
#define HAVE_TIME_H 1
#define HAVE_UNISTD_H 1
#define HAVE_PTHREAD_H 1
#define HAVE_SPAWN_H 1
#define HAVE_DIRENT_H 1
#define HAVE_LIBGEN_H 1
#define HAVE_GETOPT_H 1
#define HAVE_POLL_H 1
#define HAVE_SCHED_H 1
#define HAVE_SYS_SOCKET_H 1
#define HAVE_SYS_IOCTL_H 1
#define HAVE_SYS_SELECT_H 1
#define HAVE_SYS_PARAM_H 1
#define HAVE_SYS_RESOURCE_H 1
#define HAVE_SYSLOG_H 1
#define HAVE_PWD_H 1
#define HAVE_GRP_H 1
#define HAVE_TERMIOS_H 1
#define HAVE_DLFCN_H 1
#define HAVE_REGEX_H 1
#define HAVE_SYS_MMAN_H 1
#define HAVE_SYS_UN_H 1
#define HAVE_NET_IF_H 1
#define HAVE_PATHS_H 1
#define HAVE_PTY_H 1
#define HAVE_SHLWAPI_H 1
#define HAVE_STDARG_H 1
#define HAVE_SYS_UTSNAME_H 1

/* -- functions ---------------------------------------------------- */
#define HAVE_ATEXIT 1
#define HAVE_ASPRINTF 1
#define HAVE_VASPRINTF 1
#define HAVE_SNPRINTF 1
#define HAVE_VSNPRINTF 1
#define HAVE_STRNLEN 1
#define HAVE_STRNDUP 1
#define HAVE_STRCASECMP 1
#define HAVE_STRNCASECMP 1
#define HAVE_STRERROR_R 1
#define HAVE_STRSIGNAL 1
#define HAVE_STRSEP 1
#define HAVE_STRTOK_R 1
#define HAVE_SETENV 1
#define HAVE_UNSETENV 1
#define HAVE_PUTENV 1
#define HAVE_GETENV 1
#define HAVE_GETCWD 1
#define HAVE_MKSTEMP 1
#define HAVE_FILENO 1
#define HAVE_ISATTY 1
#define HAVE_USLEEP 1
#define HAVE_SLEEP 1
#define HAVE_ACCESS 1
#define HAVE_CHDIR 1
#define HAVE_UMASK 1
#define HAVE_STAT 1
#define HAVE_FSTAT 1
#define HAVE_LSTAT 1
#define HAVE_MKDIR 1
#define HAVE_RMDIR 1
#define HAVE_UNLINK 1
#define HAVE_WAITPID 1
#define HAVE_FORK 1
#define HAVE_EXECVE 1
#define HAVE_PIPE 1
#define HAVE_DUP 1
#define HAVE_DUP2 1
#define HAVE_CLOSE 1
#define HAVE_READ 1
#define HAVE_WRITE 1
#define HAVE_OPEN 1
#define HAVE_LSEEK 1
#define HAVE_FSYNC 1
#define HAVE_GETPID 1
#define HAVE_GETPPID 1
#define HAVE_GETHOSTNAME 1
#define HAVE_SOCKETPAIR 1
#define HAVE_FCNTL 1
#define HAVE_SELECT 1
#define HAVE_POLL 1
#define HAVE_GETIFADDRS 1
#define HAVE_INET_PTON 1
#define HAVE_INET_NTOP 1
#define HAVE_INET_ATON 1
#define HAVE_GETPAGESIZE 1
#define HAVE_SYSCONF 1
#define HAVE_MMAP 1
#define HAVE_MUNMAP 1
#define HAVE_GETTIMEOFDAY 1
#define HAVE_CLOCK_GETTIME 1
#define HAVE_GETEUID 1
#define HAVE_GETUID 1
#define HAVE_GETGID 1
#define HAVE_GETEGID 1
#define HAVE_GETGRNAM 1
#define HAVE_GETGRGID 1
#define HAVE_GETPWNAM 1
#define HAVE_GETPWUID 1
#define HAVE_LOCALTIME_R 1
#define HAVE_GMTIME_R 1
#define HAVE_CTIME_R 1
#define HAVE_ASCTIME_R 1
#define HAVE_BASENAME 1
#define HAVE_DIRNAME 1
#define HAVE_DECLHOSTNAME 1
#define HAVE_PTHREAD_SETAFFINITY_NP 0
#define HAVE_SCHED_SETAFFINITY 0
#define HAVE_SCHED_YIELD 1
#define HAVE_REGEXEC 1
#define HAVE_REGCOMP 1
#define HAVE_REGFREE 1
#define HAVE_POSIX_OPENPT 1
#define HAVE_GETPT 1
#define HAVE_PTSNAME 1
#define HAVE_GRANTPT 1
#define HAVE_UNLOCKPT 1
#define HAVE_OPENPTY 1
#define HAVE_FORKPTY 1
#define HAVE_POSIX_MEMALIGN 0
#define HAVE_POSIX_FALLOCATE 0
#define HAVE_FSEEKO 1
#define HAVE_FSEEKO64 0
#define HAVE_STRCASESTR 1
#define HAVE_STRLCAT 1
#define HAVE_STRLCPY 1
/* HAVE_GETPEEREID / HAVE_GETPEERUCRED are deliberately left UNDEFINED:
 * pmix_getid.c selects its route with defined(), not #if -- defining
 * them to 0 would compile the absent functions anyway.  Windows has no
 * peer-credential API, so pmix_util_getid reports NOT_SUPPORTED. */
#define HAVE_SETPGID 1
#define HAVE_TCGETPGRP 1
#define HAVE_SETSID 1
#define HAVE_DECL_GAI_STRERROR 1
#define HAVE_SYS_STATVFS_H 1
#define HAVE_STATVFS 1

/* -- feature selection -------------------------------------------- */
#define PMIX_ENABLE_DEBUG 0
#define PMIX_DEBUG 0
#define PMIX_ENABLE_TIMING 0
#define PMIX_ENABLE_IPV6 1
#define PMIX_ENABLE_PTY_SUPPORT 0
#define PMIX_HAVE_PDL_SUPPORT 1
#define PMIX_WANT_PRIMARY_REFCOUNT 1
#define PMIX_SUPPORT_SYSLOG 0
#define PMIX_PTL_TCP_LISTEN_BACKLOG 128
#define PMIX_DEFAULT_KEEPALIVE_TIME 0
#define PMIX_MAX_ERROR_STRING 256
#define PMIX_ENABLE_EARLY_TIMEOUT 0
#define PMIX_TESTBUILD 0
#define PMIX_ENABLE_LTO 0
#define PMIX_HAVE_ATTRIBUTE_DESTRUCTOR_FUNC 0
#define PMIX_SHOW_LOAD_ERRORS_DEFAULT 0
#define PMIX_WANT_PRETTY_PRINT 1
#define PMIX_WANT_Q 1
#define PMIX_ERROR_LOG_INTERNAL_ERRORS 0
#define PMIX_AGGREGATE_MCA_PARAMS 0

/* info_support provenance strings (autotools records the configure
 * invocation; the CMake build just describes itself) */
#define PMIX_CONFIGURE_USER "@_pmix_build_user@"
#define PMIX_CONFIGURE_DATE "@_pmix_build_date@"
#define PMIX_CONFIGURE_HOST "@_pmix_build_host@"
#define PMIX_CONFIGURE_CLI "cmake"
#define PMIX_BUILD_USER "@_pmix_build_user@"
#define PMIX_BUILD_DATE "@_pmix_build_date@"
#define PMIX_BUILD_HOST "@_pmix_build_host@"
#define PMIX_CC_ABSOLUTE "cl"
#define PMIX_BUILD_CFLAGS ""
#define PMIX_BUILD_LDFLAGS ""
#define PMIX_BUILD_LIBS ""
#define PMIX_STD_VERSION "@pmix_std_version@"
#define PMIX_STD_ABI_STABLE_VERSION "@pmix_std_abi_stable@"
#define PMIX_STD_ABI_PROVISIONAL_VERSION "@pmix_std_abi_provisional@"
#define PMIX_PROXY_VERSION_STRING PMIX_VERSION
#define PMIX_PROXY_BUGREPORT_STRING PACKAGE_BUGREPORT

/* pmix pid type */
#define PMIX_PID_T int
#define pmix_pid_t int
#define PMIX_HAVE_WINDOWS_H 1

/* aggregate/large type maxes */
#define PMIX_INT128_HAVE 0

#include "src/include/pmix_config_bottom.h"

#endif /* PMIX_CONFIG_H */
