/* -*- Mode: C; c-basic-offset:4 ; indent-tabs-mode:nil -*- */
/*
 * opal_config.h template for the native Windows (MSVC/CMake) build.
 *
 * The autotools build generates this header with autoheader; for the
 * Windows port the feature answers are fixed by the opal/win32
 * compatibility layer, so this file is configured with a small set of
 * @VAR@ substitutions for version/install paths and everything else
 * spelled out literally.
 */

#ifndef OPAL_CONFIG_H
#define OPAL_CONFIG_H

#include "opal_config_top.h"

/* -- version ------------------------------------------------------ */
#define OPAL_VERSION "@OPAL_VERSION@"
#define OPAL_MAJOR_VERSION @OMPI_MAJOR_VERSION@
#define OPAL_MINOR_VERSION @OMPI_MINOR_VERSION@
#define OPAL_RELEASE_VERSION @OMPI_RELEASE_VERSION@
#define OPAL_GREEK_VERSION "@OMPI_GREEK_VERSION@"
#define OMPI_MAJOR_VERSION @OMPI_MAJOR_VERSION@
#define OMPI_MINOR_VERSION @OMPI_MINOR_VERSION@
#define OMPI_RELEASE_VERSION @OMPI_RELEASE_VERSION@
#define OMPI_GREEK_VERSION "@OMPI_GREEK_VERSION@"
#define OMPI_VERSION "@OMPI_VERSION@"
#define OPAL_SVN_REPO_REV "@OPAL_SVN_REPO_REV@"
#define OPAL_REPO_REV "@OPAL_SVN_REPO_REV@"
#define OPAL_RELEASE_DATE "@OPAL_RELEASE_DATE@"
#define OPAL_ARCH "@OPAL_ARCH@"
#define OPAL_CONFIGURE_HOST "@OPAL_CONFIGURE_HOST@"

#define PACKAGE_NAME "@PACKAGE_NAME@"
#define PACKAGE_STRING "@PACKAGE_STRING@"
#define PACKAGE_TARNAME "@PACKAGE_TARNAME@"
#define PACKAGE_VERSION "@PACKAGE_VERSION@"
#define PACKAGE_BUGREPORT "@PACKAGE_BUGREPORT@"
#define PACKAGE_URL "@PACKAGE_URL@"

#define OPAL_PACKAGE_STRING "@PACKAGE_STRING@"
#define OPAL_IDENT_STRING "@PACKAGE_STRING@"

/* -- install directories ------------------------------------------ */
#define OPAL_PREFIX "@CMAKE_INSTALL_PREFIX@"
#define OPAL_BINDIR "@OPAL_BINDIR@"
#define OPAL_SBINDIR "@OPAL_SBINDIR@"
#define OPAL_LIBEXECDIR "@OPAL_LIBEXECDIR@"
#define OPAL_DATAROOTDIR "@OPAL_DATAROOTDIR@"
#define OPAL_DATADIR "@OPAL_DATADIR@"
#define OPAL_SYSCONFDIR "@OPAL_SYSCONFDIR@"
#define OPAL_SHAREDSTATEDIR "@OPAL_SHAREDSTATEDIR@"
#define OPAL_LOCALSTATEDIR "@OPAL_LOCALSTATEDIR@"
#define OPAL_LIBDIR "@OPAL_LIBDIR@"
#define OPAL_INCLUDEDIR "@OPAL_INCLUDEDIR@"
#define OPAL_INFODIR "@OPAL_INFODIR@"
#define OPAL_MANDIR "@OPAL_MANDIR@"
#define OPAL_PKGDATADIR "@OPAL_PKGDATADIR@"
#define OPAL_PKGLIBDIR "@OPAL_PKGLIBDIR@"
#define OPAL_PKGINCLUDEDIR "@OPAL_PKGINCLUDEDIR@"
#define OPAL_DEFAULT_PKGUPDATEDIR "@OPAL_DEFAULT_PKGUPDATEDIR@"

/* library naming */
#define OPAL_LIB_NAME "open-pal"
#define OPAL_PARAM_DEFAULT_FILES_PREFIX "@OPAL_PARAM_DEFAULT_FILES_PREFIX@"
#define OPAL_PARAM_FROM_FILE 1

/* -- compiler / arch ---------------------------------------------- */
#define OPAL_HAVE_WINDOWS 1
#define OPAL_C_HAVE_VISIBILITY 0
#define OPAL_C_HAVE_BUILTIN_EXPECT 0
#define OPAL_C_HAVE_BUILTIN_PREFETCH 0
#define OPAL_C_HAVE_BUILTIN_CLZ 0
#define OPAL_C_HAVE_BUILTIN_CTZ 0
#define OPAL_C_HAVE_BUILTIN_POPCOUNT 0
#define OPAL_C_HAVE_BUILTIN_BSWAP16 0
#define OPAL_C_HAVE_BUILTIN_BSWAP32 0
#define OPAL_C_HAVE_BUILTIN_BSWAP64 0
#define OPAL_C_HAVE_BUILTIN_ATOMIC 0
#define OPAL_C_HAVE__THREAD_LOCAL 0
#define OPAL_C_HAVE__GENERIC 0
#define OPAL_C_HAVE_ATOMIC_GT_PTR 0
#define OPAL_C_GCC_INLINE_ASSEMBLY 0
#define OPAL_C_HAVE_VA_COPY 1
#define OPAL_C_HAVE_UNDERSCORE_VA_COPY 1
#define OPAL_HAVE_SYS_TIMER_GET_CYCLES 0
#define OPAL_HAVE_SYS_TIMER_GET_FREQ 0
#define OPAL_HAVE_SYS_TIMER_IS_MONOTONIC 0

/* atomics: MSVC _Interlocked* backend */
#define OPAL_USE_ASM_ATOMICS 1
#define OPAL_USE_C11_ATOMICS 0
#define OPAL_USE_GCC_BUILTIN_ATOMICS 0
#define OPAL_HAVE_C11_CSWAP_INT128 0
#define OPAL_HAVE_SYNC_BUILTIN_CSWAP_INT128 0
#define OPAL_HAVE_GCC_BUILTIN_CSWAP_INT128 0
#define OPAL_HAVE_CMPXCHG16B 0
/* NOTE: opal_stdint.h tests these with #ifdef -- leave UNDEFINED.
 * MSVC has no 128-bit integer type, so HAVE_OPAL_INT128_T stays 0. */
/* #undef HAVE_INT128_T */
/* #undef HAVE___INT128 */
/* #undef HAVE_UINT128_T */
/* #undef HAVE_OPAL_INT128_T */
/* NOTE: opal_config_bottom.h tests these with #ifdef -- leave UNDEFINED.
 * MSVC has no _Float16/short-float type. */
/* #undef HAVE_OPAL_SHORT_FLOAT_T */
/* #undef HAVE_OPAL_SHORT_FLOAT_COMPLEX_T */
/* #undef HAVE_SHORT_FLOAT */
/* #undef HAVE_SHORT_FLOAT__COMPLEX */
/* #undef HAVE__FLOAT128 */
/* #undef HAVE___FLOAT128 */
/* #undef HAVE__FLOAT128__COMPLEX */
/* #undef HAVE___FLOAT128__COMPLEX */
/* attributes: MSVC supports essentially none of the GNU set */
#define OPAL_HAVE_ATTRIBUTE_ALIGNED 0
#define OPAL_HAVE_ATTRIBUTE_ALIGNED_MAX 0
#define OPAL_HAVE_ATTRIBUTE_ALWAYS_INLINE 0
#define OPAL_HAVE_ATTRIBUTE_COLD 0
#define OPAL_HAVE_ATTRIBUTE_CONST 0
#define OPAL_HAVE_ATTRIBUTE_CONSTRUCTOR 0
#define OPAL_HAVE_ATTRIBUTE_DEPRECATED 0
#define OPAL_HAVE_ATTRIBUTE_DESTRUCTOR 0
#define OPAL_HAVE_ATTRIBUTE_EXTENSION 0
#define OPAL_HAVE_ATTRIBUTE_FORMAT 0
#define OPAL_HAVE_ATTRIBUTE_FORMAT_FUNCPTR 0
#define OPAL_HAVE_ATTRIBUTE_HOT 0
#define OPAL_HAVE_ATTRIBUTE_MALLOC 0
#define OPAL_HAVE_ATTRIBUTE_MAY_ALIAS 0
#define OPAL_HAVE_ATTRIBUTE_NO_INSTRUMENT_FUNCTION 0
#define OPAL_HAVE_ATTRIBUTE_NOINLINE 0
#define OPAL_HAVE_ATTRIBUTE_NONNULL 0
#define OPAL_HAVE_ATTRIBUTE_NORETURN 0
#define OPAL_HAVE_ATTRIBUTE_NORETURN_FUNCPTR 0
#define OPAL_HAVE_ATTRIBUTE_OPTNONE 0
#define OPAL_HAVE_ATTRIBUTE_PACKED 0
#define OPAL_HAVE_ATTRIBUTE_PURE 0
#define OPAL_HAVE_ATTRIBUTE_SENTINEL 0
#define OPAL_HAVE_ATTRIBUTE_UNUSED 0
#define OPAL_HAVE_ATTRIBUTE_VISIBILITY 0
#define OPAL_HAVE_ATTRIBUTE_WARN_UNUSED_RESULT 0
#define OPAL_HAVE_ATTRIBUTE_WEAK_ALIAS 0
#define OPAL_HAVE_WEAK_ALIASES 0
#define OPAL_HAVE_WEAK_SYMBOLS 0
#define OPAL_HAVE_THREAD_LOCAL 1
/* MSVC supports _Thread_local since VS2019 */
#define OPAL_HAVE___CLEAR_CACHE 0
#define OPAL_HAVE___CURBRK 0
#define OPAL_HAVE___MMAP 0
#define OPAL_HAVE___SYSCALL 0

/* user-visible string length limits (config/opal_configure_options.m4
 * defaults); OMPI derives MPI_MAX_* from the same values */
#define OPAL_MAX_PROCESSOR_NAME 256
#define OPAL_MAX_ERROR_STRING   256
#define OPAL_MAX_OBJECT_NAME    64
#define OPAL_MAX_INFO_KEY       36
#define OPAL_MAX_INFO_VAL       256
#define OPAL_MAX_PORT_NAME      1024
#define OPAL_MAX_DATAREP_STRING 128
/* MPI Forum standard ABI value (docs/mpi-standard-5.0-abi.json) */
#define OMPI_MPI_MAX_OBJECT_NAME_ABI 128

/* opal_count_t backing type (config/opal_find_count_type.m4): widest
 * of {long long, long, int} that fits in size_t and covers ptrdiff_t.
 * On Win64 LLP64 that's "long long" (8 bytes). */
#define OPAL_COUNT_TYPE long long
#define OPAL_COUNT_SIZE 8
#define OPAL_COUNT_MAX  0x7fffffffffffffffll

/* -- type sizes (x86_64 Windows/LLP64) ----------------------------- */
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
#define SIZEOF_VOID_STAR 8
#define SIZEOF_FLOAT 4
#define SIZEOF_DOUBLE 8
#define SIZEOF_LONG_DOUBLE 8
#define SIZEOF_PID_T 4
#define SIZEOF_OFF_T 4
#define SIZEOF_BOOL 1
#define SIZEOF__BOOL 1
#define SIZEOF_WEAK_SYMBOLS 0

#define SIZEOF_SIGSET_T 8
#define SIZEOF_UID_T 4
#define SIZEOF_GID_T 4
#define SIZEOF_MODE_T 2

/* type existence */
#define HAVE_INTPTR_T 1
#define HAVE_UINTPTR_T 1
#define HAVE_INT64_T 1
#define HAVE_UINT64_T 1
#define HAVE_INTPTR_T 1
#define HAVE_LONG_LONG 1
#define OPAL_HAVE_LONG_LONG 1
#define HAVE_SOCKLEN_T 1
#define HAVE_SIGINFO_T 1

/* type alignments (x86_64 MSVC) */
#define OPAL_ALIGNMENT_BOOL 1
#define OPAL_ALIGNMENT_CHAR 1
#define OPAL_ALIGNMENT_SHORT 2
#define OPAL_ALIGNMENT_WCHAR 2
#define OPAL_ALIGNMENT_INT 4
#define OPAL_ALIGNMENT_LONG 4
#define OPAL_ALIGNMENT_LONG_LONG 8
#define OPAL_ALIGNMENT_FLOAT 4
#define OPAL_ALIGNMENT_DOUBLE 8
#define OPAL_ALIGNMENT_LONG_DOUBLE 8
#define OPAL_ALIGNMENT_INT8 1
#define OPAL_ALIGNMENT_INT16 2
#define OPAL_ALIGNMENT_INT32 4
#define OPAL_ALIGNMENT_INT64 8
#define OPAL_ALIGNMENT_INT128 16
#define OPAL_ALIGNMENT_SIZE_T 8
#define OPAL_ALIGNMENT_VOID_P 8
#define OPAL_ALIGNMENT_POINTER 8
#define OPAL_ALIGNMENT_FLOAT_COMPLEX 4
#define OPAL_ALIGNMENT_DOUBLE_COMPLEX 8
#define OPAL_ALIGNMENT_LONG_DOUBLE_COMPLEX 8
#define OPAL_ALIGNMENT_SHORT_FLOAT 2
#define OPAL_ALIGNMENT_SHORT_FLOAT_COMPLEX 2
#define OPAL_ALIGNMENT_OPAL_SHORT_FLOAT_T 2
#define OPAL_ALIGNMENT__FLOAT128 16
#define OPAL_ALIGNMENT___FLOAT128 16
#define OPAL_ALIGNMENT__FLOAT128_COMPLEX 16
#define OPAL_ALIGNMENT___FLOAT128_COMPLEX 16

/* -- headers ------------------------------------------------------- */
#define HAVE_ALLOCA_H 1
#define HAVE_ARPA_INET_H 1
#define HAVE_ASSERT_H 1
/* #undef HAVE_CRT_EXTERNS_H */
#define HAVE_CTYPE_H 1
#define HAVE_DIRENT_H 1
#define HAVE_DLFCN_H 1
#define HAVE_ENDIAN_H 1
#define HAVE_ERRNO_H 1
#define HAVE_EXECINFO_H 1
#define HAVE_FCNTL_H 1
#define HAVE_FLOAT_H 1
#define HAVE_FNMATCH_H 1
#define HAVE_GETOPT_H 1
#define HAVE_GRP_H 1
/* #undef HAVE_HOSTLIB_H */
/* #undef HAVE_IEEE754_H */
#define HAVE_IFADDRS_H 1
#define HAVE_INTTYPES_H 1
/* #undef HAVE_IOLIB_H */
#define HAVE_LIBGEN_H 1
#define HAVE_LIMITS_H 1
/* #undef HAVE_LINUX_ETHTOOL_H */
/* #undef HAVE_LINUX_KCMP_H */
/* #undef HAVE_LINUX_MMAN_H */
/* #undef HAVE_LINUX_SOCKIOS_H */
#define HAVE_MALLOC_H 1
#define HAVE_MATH_H 1
/* #undef HAVE_MNTENT_H */
#define HAVE_NETDB_H 1
#define HAVE_NETINET_IN_H 1
#define HAVE_NETINET_TCP_H 1
#define HAVE_NET_IF_H 1
/* #undef HAVE_NET_UIO_H */
#define HAVE_PATHS_H 1
#define HAVE_POLL_H 1
/* #undef HAVE_POSIX_FALLOCATE */
#define HAVE_PTHREAD_H 1
#define HAVE_PWD_H 1
/* #undef HAVE_REGEX_H */
#define HAVE_SCHED_H 1
#define HAVE_SEMAPHORE_H 1
/* #undef HAVE_SHLWAPI_H */
#define HAVE_SIGNAL_H 1
/* #undef HAVE_SOCKLIB_H */
#define HAVE_SPAWN_H 1
#define HAVE_STDARG_H 1
#define HAVE_STDBOOL_H 1
#define HAVE_STDDEF_H 1
#define HAVE_STDINT_H 1
#define HAVE_STDIO_H 1
#define HAVE_STDLIB_H 1
#define HAVE_STRINGS_H 1
#define HAVE_STRING_H 1
#define HAVE_SYSLOG_H 1
/* #undef HAVE_SYS_FCNTL_H */
#define HAVE_SYS_IOCTL_H 1
#define HAVE_SYS_IPC_H 1
#define HAVE_SYS_MMAN_H 1
/* #undef HAVE_SYS_MOUNT_H */
#define HAVE_SYS_PARAM_H 1
/* #undef HAVE_SYS_PRCTL_H */
#define HAVE_SYS_RESOURCE_H 1
#define HAVE_SYS_SELECT_H 1
#define HAVE_SYS_SHM_H 1
#define HAVE_SYS_SOCKET_H 1
/* #undef HAVE_SYS_SOCKIO_H */
/* #undef HAVE_SYS_STATFS_H */
#define HAVE_SYS_STATVFS_H 1
#define HAVE_SYS_STAT_H 1
#define HAVE_SYS_SYSCALL_H 1
#define HAVE_SYS_TIMES_H 1
#define HAVE_SYS_TIME_H 1
#define HAVE_SYS_TYPES_H 1
/* #undef HAVE_SYS_UCRED_H */
#define HAVE_SYS_UIO_H 1
#define HAVE_SYS_UN_H 1
#define HAVE_SYS_UTSNAME_H 1
/* #undef HAVE_SYS_VFS_H */
#define HAVE_SYS_WAIT_H 1
#define HAVE_TERMIOS_H 1
#define HAVE_TIME_H 1
#define HAVE_UNISTD_H 1
#define HAVE_WCHAR_H 1
/* #undef HAVE_XPMEM_H */
/* #undef HAVE_SN_XPMEM_H */
/* #undef HAVE_LIBZ */
/* -- library functions (through the compat layer or the CRT) ------- */
#define HAVE_ACCESS 1
#define HAVE_ASPRINTF 1
#define HAVE_ATEXIT 1
/* #undef HAVE_BUILTIN_EXPECT */
/* #undef HAVE_BUILTIN_PREFETCH */
/* #undef HAVE_CKACTION */
#define HAVE_CLOCK_GETTIME 1
#define OPAL_HAVE_CLOCK_GETTIME 1
#define HAVE_CLOSE 1
#define HAVE_CTIME_R 1
#define HAVE_DIRNAME 1
#define OPAL_HAVE_DIRNAME 1
#define HAVE_DUP 1
#define HAVE_DUP2 1
#define HAVE_EXECVE 1
#define HAVE_FCHMOD 1
#define HAVE_FCNTL 1
#define HAVE_FLOCK 1
/* #undef HAVE_FORK */
#define HAVE_FSTAT 1
#define HAVE_GETHOSTBYNAME 1
#define HAVE_GETIFADDRS 1
#define HAVE_GETPAGESIZE 1
/* #undef HAVE_GETPEEREID */
#define HAVE_GETPWUID 1
/* #undef HAVE_GETRUSAGE */
#define HAVE_GETTIMEOFDAY 1
#define HAVE_ISATTY 1
#define HAVE_KILL 1
#define HAVE_LOCALTIME_R 1
#define HAVE_LSTAT 1
#define HAVE_MALLOC 1
#define HAVE_MEMMOVE 1
#define HAVE_MKDIR 1
#define HAVE_MKFIFO 1
#define HAVE_MMAP 1
#define HAVE_MUNMAP 1
#define HAVE_NANOSLEEP 1
#define HAVE_PIPE 1
#define HAVE_POSIX_MEMALIGN 1
#define HAVE_PUTENV 1
/* #undef HAVE_REGEXEC */
#define HAVE_SCHED_YIELD 1
#define HAVE_SETENV 1
/* #undef HAVE_SETSID */
#define HAVE_SIGACTION 1
/* #undef HAVE_SIGQUEUE */
#define HAVE_SLEEP 1
#define HAVE_SNPRINTF 1
#define HAVE_SOCKET 1
/* #undef HAVE_STATFS */
#define HAVE_STATVFS 1
#define HAVE_STRDUP 1
#define HAVE_STRERROR 1
#define HAVE_STRSIGNAL 1
#define HAVE_STRTOL 1
#define HAVE_STRTOLL 1
#define HAVE_SYSLOG 1
#define HAVE_SYSCONF 1
#define HAVE_TIMES 1
#define HAVE_UNAME 1
#define HAVE_USLEEP 1
#define HAVE_UTIME 1
#define HAVE_UTIMES 1
#define HAVE_VASPRINTF 1
#define HAVE_VSNPRINTF 1
#define HAVE_WAITPID 1
/* #undef HAVE__NSGETENVIRON */
/* #undef HAVE_LOCAL_PEERCRED */
/* #undef HAVE_SO_PEERCRED */
/* #undef HAVE_DECL_PMIX_PACKAGE_RANK */
/* -- struct members / decls ---------------------------------------- */
#define HAVE_DECL_AF_INET6 1
#define HAVE_DECL_AF_UNSPEC 1
#define HAVE_DECL_PF_INET6 1
#define HAVE_DECL_PF_UNSPEC 1
/* #undef HAVE_DECL_OPEN_MEMSTREAM */
/* #undef HAVE_DECL_RLIMIT_AS */
/* #undef HAVE_DECL_RLIMIT_CORE */
/* #undef HAVE_DECL_RLIMIT_FSIZE */
/* #undef HAVE_DECL_RLIMIT_MEMLOCK */
/* #undef HAVE_DECL_RLIMIT_NOFILE */
/* #undef HAVE_DECL_RLIMIT_NPROC */
/* #undef HAVE_DECL_RLIMIT_STACK */
/* #undef HAVE_DECL_ETHTOOL_CMD_SPEED */
/* #undef HAVE_DECL_SIOCETHTOOL */
#define HAVE_DECL___FUNC__ 1
/* #undef HAVE_SIGINFO_T_SI_BAND */
/* #undef HAVE_SIGINFO_T_SI_FD */
/* #undef HAVE_STRUCT_ETHTOOL_CMD */
/* #undef HAVE_STRUCT_ETHTOOL_CMD_SPEED_HI */
/* #undef HAVE_STRUCT_IFREQ */
/* #undef HAVE_STRUCT_IFREQ_IFR_HWADDR */
/* #undef HAVE_STRUCT_IFREQ_IFR_MTU */
#define HAVE_STRUCT_SOCKADDR_IN 1
#define HAVE_STRUCT_SOCKADDR_IN6 1
/* #undef HAVE_STRUCT_SOCKADDR_SA_LEN */
#define HAVE_STRUCT_SOCKADDR_STORAGE 1
/* #undef HAVE_STRUCT_SOCKADDR_UN */
/* #undef HAVE_STRUCT_STATFS_F_FSTYPENAME */
/* #undef HAVE_STRUCT_STATFS_F_TYPE */
/* #undef HAVE_STRUCT_STATVFS_F_BASETYPE */
/* #undef HAVE_STRUCT_STATVFS_F_FSTYPENAME */
#define HAVE_STRUCT_TIMESPEC_TV_NSEC 1
#define HAVE_STRUCT_TIMEVAL 1
#define HAVE_STRUCT_TIMESPEC 1
#define HAVE_STRUCT_FD_SET 1
/* #undef HAVE_STRUCT_DIRENT_D_TYPE */
/* #undef HAVE_STRUCT_STAT_ST_BLKSIZE */
/* #undef HAVE_STRUCT_STAT_ST_RDEV */
#define HAVE_UNIX_BYTESWAP 1

/* -- feature options (fixed for the Windows port) ------------------- */
#define OPAL_ENABLE_DEBUG 0
#define OPAL_ENABLE_DEBUG_RELIABILITY 0
#define OPAL_ENABLE_FT 0
#define OPAL_ENABLE_FT_MPI 0
#define OPAL_ENABLE_HETEROGENEOUS_SUPPORT 0
#define OPAL_ENABLE_IPV6 1
#define OPAL_ENABLE_MEM_DEBUG 0
#define OPAL_ENABLE_MEM_PROFILE 0
#define OPAL_ENABLE_MPI_THREADS 0
#define OPAL_ENABLE_PROGRESS_THREADS 0
#define OPAL_ENABLE_TIMING 0
#define OPAL_WANT_HOME_CONFIG_FILES 0
#define OPAL_WANT_MEMCHECKER 0
#define OPAL_WANT_PRETTY_PRINT_STACKTRACE 0
#define OPAL_WANT_SPMM_FDT_SUPPORT 0
#define OPAL_WANT_LIBLTDL 0
#define OPAL_HAVE_DL_SUPPORT 1
#define OPAL_ENABLE_PICKY_COMPILERS 0

#define OPAL_HAVE_SOLARIS 0
#define OPAL_HAVE_LINUX 0
#define OPAL_HAVE_OSX 0
#define OPAL_HAVE_POSIX_THREADS 1
#define OPAL_THREADS_HAVE_DIFFERENT_TSD 0
#define OPAL_THREADS_HAVE_NATIVE_SPINLOCKS 0
#define OPAL_ENABLE_MULTI_THREADS 0
#define OPAL_PROGRESS_USE_TIMERS 0
#define OPAL_PROGRESS_ONLY_USEC_NATIVE 0

/* hardware */
#define OPAL_CUDA_GDR_SUPPORT 0
#define OPAL_CUDA_SUPPORT 0
#define OPAL_CUDA_VMM_SUPPORT 0
#define OPAL_OFI_PCI_DATA_AVAILABLE 0
#define OPAL_ROCM_SUPPORT 0
#define OPAL_ZE_SUPPORT 0
#define OPAL_HAVE_UCT_EP_ATOMIC64_POST 0
#define OPAL_DL_LIBLTDL_HAVE_LT_DLADVISE 0
#define OPAL_BTL_USNIC_UNIT_TESTS 0

/* -- MCA build-time gorp -------------------------------------------- */
/* the frameworks/components compiled into the libraries are listed in
 * the generated static-components.h files; see cmake/*.cmake */
#define OMPI_MCA_BUILD_VERBOSE 0

/* mirrors configure.ac:1550 OPAL_SET_MCA_CMD_LINE_ID([mca]) */
#define OPAL_MCA_CMD_LINE_ID "mca"
/* mirrors configure.ac:1549 OPAL_SET_MCA_PREFIX([OMPI_MCA_]) */
#define OPAL_MCA_PREFIX "OMPI_MCA_"
/* mirrors configure.ac:187-190 -- dense 0..3 encoding consumed by
 * opal/mca/base/mca_base_event.h */
#define OPAL_MCA_BASE_CB_REQUIRE_NONE               0
#define OPAL_MCA_BASE_CB_REQUIRE_MPI_RESTRICTED     1
#define OPAL_MCA_BASE_CB_REQUIRE_THREAD_SAFE        2
#define OPAL_MCA_BASE_CB_REQUIRE_ASYNC_SIGNAL_SAFE  3
/* mirrors opal_configure_options.m4 default ("all") */
#define OPAL_SHOW_LOAD_ERRORS_DEFAULT "all"

/* threads framework: the pthreads component is selected on Windows --
 * it compiles against the opal/win32 pthread shim.  These mirror the
 * AC_DEFINEs in opal/mca/threads/pthreads/configure.m4. */
#define MCA_threads_base_include_HEADER \
    "opal/mca/threads/pthreads/threads_pthreads_threads.h"
#define MCA_threads_mutex_base_include_HEADER \
    "opal/mca/threads/pthreads/threads_pthreads_mutex.h"
#define MCA_threads_tsd_base_include_HEADER \
    "opal/mca/threads/pthreads/threads_pthreads_tsd.h"

/* timer framework: no component selected yet -- fall back to the
 * framework's built-in null timer (mirrors timer/configure.m4). */
#define MCA_timer_IMPLEMENTATION_HEADER \
    "opal/mca/timer/base/timer_base_null.h"

/* memory framework: no component selected -- mirrors the default in
 * memory/configure.m4. */
#define MCA_memory_IMPLEMENTATION_HEADER \
    "opal/mca/memory/base/empty.h"

/* help-message files generated into opal/util */
#define OPAL_PACKAGE_VERSION OPAL_VERSION

#include "opal_config_bottom.h"

#endif /* OPAL_CONFIG_H */
