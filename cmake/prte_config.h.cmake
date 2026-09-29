/* -*- Mode: C; c-basic-offset:4 ; indent-tabs-mode:nil -*- */
/*
 * prte_config.h template for the native Windows (MSVC/CMake) build.
 *
 * The autotools build generates this header with autoheader; for the
 * Windows port the feature answers are fixed by the opal/win32
 * compatibility layer (shared with open-pal and pmix), so this file
 * spells the answers out literally.
 */

#ifndef PRTE_CONFIG_H
#define PRTE_CONFIG_H

#include "src/include/prte_config_top.h"

/* -- version ------------------------------------------------------ */
#define PRTE_MAJOR_VERSION @prtemajor@
#define PRTE_MINOR_VERSION @prteminor@
#define PRTE_RELEASE_VERSION @prterelease@
#define PRTE_GREEK_VERSION "@prtegreek@"
#define PRTE_REPO_REV "@prtereporev@"
#define PRTE_RELEASE_DATE "@prterelease_date@"
#define PRTE_VERSION "@prteversion@"
#define PRTE_PACKAGE_STRING "PRRTE @prteversion@"
#define PRTE_IDENT_STRING PRTE_PACKAGE_STRING

#define PACKAGE_NAME "prrte"
#define PACKAGE_STRING PRTE_PACKAGE_STRING
#define PACKAGE_TARNAME "prrte"
#define PACKAGE_VERSION "@prteversion@"
#define PACKAGE_BUGREPORT "https://github.com/openpmix/prrte/issues"
#define PACKAGE_URL "https://docs.prrte.org/"

/* -- install directories ------------------------------------------ */
#define PRTE_BINDIR "@PRTE_BINDIR@"
#define PRTE_SBINDIR "@PRTE_SBINDIR@"
#define PRTE_LIBEXECDIR "@PRTE_LIBEXECDIR@"
#define PRTE_DATAROOTDIR "@PRTE_DATAROOTDIR@"
#define PRTE_DATADIR "@PRTE_DATADIR@"
#define PRTE_SYSCONFDIR "@PRTE_SYSCONFDIR@"
#define PRTE_SHAREDSTATEDIR "@PRTE_SHAREDSTATEDIR@"
#define PRTE_LOCALSTATEDIR "@PRTE_LOCALSTATEDIR@"
#define PRTE_LIBDIR "@PRTE_LIBDIR@"
#define PRTE_INCLUDEDIR "@PRTE_INCLUDEDIR@"
#define PRTE_INFODIR "@PRTE_INFODIR@"
#define PRTE_MANDIR "@PRTE_MANDIR@"
#define PRTE_PKGDATADIR "@PRTE_PKGDATADIR@"
#define PRTE_PKGLIBDIR "@PRTE_PKGLIBDIR@"
#define PRTE_PKGINCLUDEDIR "@PRTE_PKGINCLUDEDIR@"

#define PRTE_PARAM_DEFAULT_FILES_PREFIX "@PRTE_BINDIR@/.."
#define PRTE_WANT_HOME_CONFIG_FILES 1
#define PRTE_SET_USEABLE 1

/* -- compiler ----------------------------------------------------- */
#define PRTE_CC "cl"
#define PRTE_HAVE_ATTRIBUTE_ALIGNED 0
#define PRTE_HAVE_ATTRIBUTE_ALIGNED_FUNCPTR 0
#define PRTE_HAVE_ATTRIBUTE_ALWAYS_INLINE 0
#define PRTE_HAVE_ATTRIBUTE_COLD 0
#define PRTE_HAVE_ATTRIBUTE_CONST 0
#define PRTE_HAVE_ATTRIBUTE_CONSTRUCTOR 0
#define PRTE_HAVE_ATTRIBUTE_DEPRECATED 0
#define PRTE_HAVE_ATTRIBUTE_DEPRECATED_ARGUMENT 0
#define PRTE_HAVE_ATTRIBUTE_DESTRUCTOR 0
#define PRTE_HAVE_ATTRIBUTE_ERROR 0
#define PRTE_HAVE_ATTRIBUTE_EXTENSION 0
#define PRTE_HAVE_ATTRIBUTE_FORMAT 0
#define PRTE_HAVE_ATTRIBUTE_FORMAT_FUNCPTR 0
#define PRTE_HAVE_ATTRIBUTE_HOT 0
#define PRTE_HAVE_ATTRIBUTE_MALLOC 0
#define PRTE_HAVE_ATTRIBUTE_MAY_ALIAS 0
#define PRTE_HAVE_ATTRIBUTE_NO_INSTRUMENT_FUNCTION 0
#define PRTE_HAVE_ATTRIBUTE_NO_SANITIZE 0
#define PRTE_HAVE_ATTRIBUTE_NO_SANITIZE_ADDRESS 0
#define PRTE_HAVE_ATTRIBUTE_NO_SANITIZE_THREAD 0
#define PRTE_HAVE_ATTRIBUTE_NO_SANITIZE_UNDEFINED 0
#define PRTE_HAVE_ATTRIBUTE_NOINLINE 0
#define PRTE_HAVE_ATTRIBUTE_NONNULL 0
#define PRTE_HAVE_ATTRIBUTE_NORETURN 0
#define PRTE_HAVE_ATTRIBUTE_NORETURN_FUNCPTR 0
#define PRTE_HAVE_ATTRIBUTE_OPTNONE 0
#define PRTE_HAVE_ATTRIBUTE_PACKED 0
#define PRTE_HAVE_ATTRIBUTE_PURE 0
#define PRTE_HAVE_ATTRIBUTE_SENTINEL 0
#define PRTE_HAVE_ATTRIBUTE_UNUSED 0
#define PRTE_HAVE_ATTRIBUTE_VISIBILITY 0
#define PRTE_C_HAVE_VISIBILITY 0
#define PRTE_HAVE_ATTRIBUTE_WARN_UNUSED_RESULT 0
#define PRTE_HAVE_ATTRIBUTE_WEAK_ALIAS 0
#define PRTE_HAVE_WEAK_SYMBOLS 0
#define PRTE_HAVE_VA_COPY 1
#define PRTE_HAVE_UNDERSCORE_VA_COPY 1
#define PRTE_PICKY_COMPILERS 0

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
#define PRTE_PTRDIFF_TYPE ptrdiff_t
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
#define HAVE_FILENO_UNLOCKED 1
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
#define HAVE_SETPGID 1
#define HAVE_TCGETPGRP 1
#define HAVE_SETSID 1
#define HAVE_DECL_GAI_STRERROR 1
#define HAVE_SYS_STATVFS_H 1
#define HAVE_STATVFS 1

/* -- feature selection -------------------------------------------- */
#define PRTE_ENABLE_DEBUG 0
#define PRTE_DEBUG 0
#define PRTE_ENABLE_IPV6 1
#define PRTE_ENABLE_PTY_SUPPORT 0
#define PRTE_ENABLE_GETPWUID 0
#define PRTE_ENABLE_TIMING 0
#define PRTE_WANT_PRETTY_PRINT_STACKTRACE 0
#define PRTE_WANT_PRTE_PREFIX_BY_DEFAULT 0
#define PRTE_WANT_LEGACY_TOOLS 1
#define PRTE_TESTBUILD 0
#define PRTE_TESTBUILD_LAUNCHERS 0
#define PRTE_BINARY_PREFIX ""
#define PRTE_HAVE_DVM_MOD_EVENTS 1
#define PRTE_HAVE_SCHED_SETAFFINITY 0
#define PRTE_HAVE_SET_MEMPOLICY 0
#define PRTE_HAVE_LINUX_PTRACE 0
#define PRTE_HAVE_LIBEV 0
#define PRTE_HAVE_LIBNL3 0
#define PRTE_HAVE_RAS_SLURM 0
#define PRTE_HAVE_SLURM_EXTENSIONS 0
#define PRTE_WANT_SLURM_EXTENSIONS 0
#define PRTE_HAVE_STOP_ON_EXEC 0
#define PRTE_HAVE_PMIXCC 0
#define PRTE_SHOW_LOAD_ERRORS_DEFAULT 0

/* -- embedded PMIx capability answers ----------------------------- */
/* The embedded openpmix is master (7.0.0a1): every capability flag
 * PRRTE probes is present.  PRTE_CHECK_PMIX_CAP checks pmix_version.h
 * for PMIX_CAP_<name>; all of the below correspond to flags this PMIx
 * sets. */
#define PRTE_PMIX_MINIMUM_VERSION 0x70000
#define PRTE_PMIX_MIN_VERSION_STRING "7.0.0"
#define PRTE_PMIX_MAX_VERSION_STRING "1000.0.0"
#define PRTE_PMIX_GET_NUMBER_FN 1
#define PRTE_PMIX_STOP_PRGTHRD 1
#define PRTE_PMIX_SERVER2_UPCALLS 1
#define PRTE_PMIX_INMEMHELP 1
#define PRTE_PMIX_HAVE_REGEX2 1
#define PRTE_PMIX_HAVE_GROUP_FT 1
#define PRTE_PMIX_CLI_QUAL_VALUE 1
#define PRTE_PMIX_MCA_FW_VERSION 1
#define PRTE_PMIX_DEVICE_ENUM 1
#define PRTE_PMIX_IOF_FILE_PATTERN 1
#define PRTE_PMIX_CLI_ORDER 1
#define PRTE_PMIX_IOF_DELIVER_LOCAL 1
#define PRTE_PMIX_IOF_FLOW_CONTROL 1
#define PRTE_PMIX_LTO_CAPABILITY 0
#define PRTE_PMIX_ALLOC_REQ 1
#define PRTE_PMIX_ALLOC_INHERITANCE 1
#define PRTE_PMIX_ALLOC_INHERIT_CHILD 1
#define PRTE_PMIX_ALLOC_ACTIVATE 1

/* -- provenance ---------------------------------------------------- */
#define PRTE_CONFIGURE_USER "@_prte_build_user@"
#define PRTE_CONFIGURE_DATE "@_prte_build_date@"
#define PRTE_CONFIGURE_HOST "@_prte_build_host@"
#define PRTE_CONFIGURE_CLI "cmake"
#define PRTE_BUILD_USER "@_prte_build_user@"
#define PRTE_BUILD_DATE "@_prte_build_date@"
#define PRTE_BUILD_HOST "@_prte_build_host@"
#define PRTE_CC_ABSOLUTE "cl"
#define PRTE_BUILD_CFLAGS ""
#define PRTE_BUILD_CPPFLAGS ""
#define PRTE_BUILD_LDFLAGS ""
#define PRTE_BUILD_LIBS ""

#include "src/include/prte_config_bottom.h"

#endif /* PRTE_CONFIG_H */
