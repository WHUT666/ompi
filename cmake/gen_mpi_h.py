#!/usr/bin/env python3
#
# Copyright (c) 2026      Amazon.com, Inc. or its affiliates.
#                         All Rights reserved.
# $COPYRIGHT$
#
# Additional copyrights may follow
#
# $HEADER$
# SPDX-License-Identifier: BSD-3-Clause-Open-MPI
#
# Generate ompi/include/mpi.h from mpi.h.in for the Windows build.
#
# mpi.h.in is an autoheader-style template: '#undef FOO' lines between the
# '@OMPI_BEGIN_CONFIGURE_SECTION@' and '@OMPI_END_CONFIGURE_SECTION@' markers
# are replaced by config.status in autotools builds.  We emulate that with a
# fixed substitution table for MSVC/x86_64 (matching what
# cmake/opal_config.h.cmake answers for the same feature tests).
#
# Logical/boolean macros are emitted as '#define X 0' (never left undefined,
# per Open MPI coding rules).

import re
import sys


# Define table for Windows x86_64 / MSVC.  Values intentionally mirror the
# answers a real configure would compute for this toolchain.
DEFINES = {
    # build compiler identity (opal_portable_platform_real.h: MSVC familyid=14)
    'OPAL_BUILD_PLATFORM_COMPILER_FAMILYID': '14',
    'OPAL_BUILD_PLATFORM_COMPILER_VERSION': '_MSC_VER',
    'OPAL_STDC_HEADERS': '1',
    # attribute support under MSVC: __opal_attribute_* macros handle this
    'OPAL_HAVE_ATTRIBUTE_DEPRECATED': '0',
    'OPAL_HAVE_ATTRIBUTE_DEPRECATED_ARGUMENT': '0',
    'OPAL_HAVE_ATTRIBUTE_ERROR': '0',
    # sys/time.h is provided by our own shim for the library build, but the
    # installed mpi.h must not require it -> 0 for consumers
    'OPAL_HAVE_SYS_TIME_H': '0',
    'OPAL_HAVE_SYS_SYNCH_H': '0',
    'OPAL_HAVE_LONG_LONG': '1',
    'OPAL_SIZEOF_BOOL': '1',
    'OPAL_SIZEOF_INT': '4',
    'OPAL_SIZEOF_VOID_P': '8',
    # string limits (mirrors cmake/opal_config.h.cmake)
    'OPAL_MAX_DATAREP_STRING': '128',
    'OPAL_MAX_ERROR_STRING': '256',
    'OPAL_MAX_INFO_KEY': '36',
    'OPAL_MAX_INFO_VAL': '256',
    'OPAL_MAX_OBJECT_NAME': '64',
    'OPAL_MAX_PORT_NAME': '1024',
    'OPAL_MAX_PROCESSOR_NAME': '256',
    'OPAL_MAX_PSET_NAME_LEN': '512',
    'OPAL_MAX_STRINGTAG_LEN': '1024',
    # no Fortran in this port: MPI_Fint still needs a concrete int type
    'OMPI_FORTRAN_STATUS_SIZE': '6',
    'OMPI_HAVE_FORTRAN_LOGICAL1': '0',
    'OMPI_HAVE_FORTRAN_LOGICAL2': '0',
    'OMPI_HAVE_FORTRAN_LOGICAL4': '0',
    'OMPI_HAVE_FORTRAN_LOGICAL8': '0',
    'OMPI_HAVE_FORTRAN_LOGICAL16': '0',
    'OMPI_HAVE_FORTRAN_INTEGER1': '0',
    'OMPI_HAVE_FORTRAN_INTEGER2': '0',
    'OMPI_HAVE_FORTRAN_INTEGER4': '0',
    'OMPI_HAVE_FORTRAN_INTEGER8': '0',
    'OMPI_HAVE_FORTRAN_INTEGER16': '0',
    'OMPI_HAVE_FORTRAN_REAL2': '0',
    'OMPI_HAVE_FORTRAN_REAL4': '0',
    'OMPI_HAVE_FORTRAN_REAL8': '0',
    'OMPI_HAVE_FORTRAN_REAL16': '0',
    'ompi_fortran_bogus_type_t': 'int',
    'ompi_fortran_integer_t': 'int',
    'OMPI_ENABLE_MPI1_COMPAT': '0',
    # no _Complex keyword in MSVC C mode
    'HAVE_FLOAT__COMPLEX': '0',
    'HAVE_DOUBLE__COMPLEX': '0',
    'HAVE_LONG_DOUBLE__COMPLEX': '0',
    # public ABI types (config/ompi_find_mpi_aint_count_offset.m4 results)
    'OMPI_MPI_AINT_TYPE': 'ptrdiff_t',
    'OMPI_MPI_OFFSET_TYPE': 'long long',
    'OMPI_OFFSET_DATATYPE': 'MPI_LONG_LONG',
    'OMPI_MPI_OFFSET_SIZE': '8',
    'OMPI_MPI_COUNT_TYPE': 'long long',
    'OMPI_PARAM_CHECK': '1',
    'OMPI_WANT_MPI_INTERFACE_WARNING': '0',
    'OPAL_C_HAVE_VISIBILITY': '0',
    # MCA event-callback safety levels (must match configure.ac values)
    'OPAL_MCA_BASE_CB_REQUIRE_NONE': '0',
    'OPAL_MCA_BASE_CB_REQUIRE_MPI_RESTRICTED': '1',
    'OPAL_MCA_BASE_CB_REQUIRE_THREAD_SAFE': '2',
    'OPAL_MCA_BASE_CB_REQUIRE_ASYNC_SIGNAL_SAFE': '3',
}

# Filled in from the --version args (read from top-level VERSION)
EXTRA = {
    'OMPI_MAJOR_VERSION': None,
    'OMPI_MINOR_VERSION': None,
    'OMPI_RELEASE_VERSION': None,
    'MPI_VERSION': None,
    'MPI_SUBVERSION': None,
}

# Internal-only OMPI config macros for the generated ompi_config.h.
# These mirror config/ompi_configure_options.m4 answers for a Windows,
# C-only, embedded-PMIx/PRRTE build.
INTERNAL_DEFINES = """\
/* Internal-only OMPI build configuration (Windows/MSVC answers) */
#define OMPI_BUILD_MPI_PROFILING 1
#define OMPI_GROUP_SPARSE 0
#define OMPI_WANT_PERUSE 0
#define OMPI_ENABLE_GREQUEST_EXTENSIONS 1
#define OMPI_USING_INTERNAL_PRRTE 1
#define OPAL_USING_INTERNAL_PMIX 1
#define OMPI_BUILD_FORTRAN_BINDINGS 0
#define OMPI_TIMING 0
#define OMPI_SIZEOF_FORTRAN_INTEGER 4
#define OMPI_MPI_COUNT_SIZE 8
#define MPI_COUNT_MAX OPAL_COUNT_MAX
#define OMPI_HAVE_FORTRAN_COMPLEX4 0
#define OMPI_HAVE_FORTRAN_COMPLEX8 0
#define OMPI_HAVE_FORTRAN_COMPLEX16 0
#define OMPI_HAVE_FORTRAN_COMPLEX32 0
#define OMPI_REAL16_MATCHES_C 0
#define OMPI_MCA_OP_HAVE_AVX 0
#define OMPI_MCA_OP_HAVE_AVX2 0
#define OMPI_MCA_OP_HAVE_AVX512 0
#define OMPI_MCA_OP_HAVE_NEON 0
#define OMPI_MCA_OP_HAVE_SVE 0
#define OMPI_MCA_OP_HAVE_RVV 0
#define OMPI_FORTRAN_CAPS 0
#define OMPI_FORTRAN_PLAIN 1
#define OMPI_FORTRAN_SINGLE_UNDERSCORE 0
#define OMPI_FORTRAN_DOUBLE_UNDERSCORE 0
#define OMPI_FORTRAN_MIXED 0
#define OMPI_MPIHANDLES_DLL_PREFIX "ompi_dbg_mpihandles"
#define OMPI_PRTERUN_PATH "prterun"

/* Endpoint tags (config/ompi_endpoint_tag.m4): pml/ob1 + bml/r2 both
 * require the BML slot; only one tag exists in this component set. */
#define OMPI_PROC_ENDPOINT_TAG_BML 0
#define OMPI_PROC_ENDPOINT_TAG_MAX 1

/* min(INT_MAX, fortran INTEGER max) -- no fortran here */
#define OMPI_FORTRAN_HANDLE_MAX 2147483647

/* MPI_PARAM_CHECK: 'runtime' -> checks consult the ompi_mpi_param_check
 * bool (matches config/ompi_configure_options.m4 default) */
#define MPI_PARAM_CHECK ompi_mpi_param_check

/* debugger-plugin names (autotools passes these via CPPFLAGS) */
#define OMPI_MSGQ_DLL "ompi_dbg_msgq.dll"
#define OMPI_MSGQ_DLL_PREFIX "ompi_dbg_msgq"
#define OMPI_MPIR_DLL_NAME "ompi_dbg_msgq.dll"
#define OMPI_REPO_REV ""
#define OMPI_RELEASE_DATE "unreleased"

/* Fortran interop typedefs -- configure-provided under autotools; the
 * C-only Windows build just needs parseable filler types. */
#define ompi_fortran_logical_t int
#define ompi_fortran_integer1_t int8_t
#define ompi_fortran_integer2_t int16_t
#define ompi_fortran_integer4_t int32_t
#define ompi_fortran_integer8_t int64_t
#define ompi_fortran_integer16_t ompi_fortran_i128_t
typedef struct { int64_t _v[2]; } ompi_fortran_i128_t;
#define ompi_fortran_real_t float
#define ompi_fortran_real2_t int16_t
#define ompi_fortran_real4_t float
#define ompi_fortran_real8_t double
#define ompi_fortran_real16_t long double
#define ompi_fortran_double_precision_t double
#define ompi_fortran_complex_t float
#define ompi_fortran_double_complex_t double
#define ompi_fortran_common_t char

/* MPI-IO / ompio feature answers for the Windows port.  The shim
 * provides POSIX semaphores (semaphore.h over CreateSemaphoreA),
 * POSIX AIO (aio.h via worker threads doing positioned I/O, see
 * opal/win32/opal_win32_aio.c) and sys/param.h+sys/stat.h, but no
 * preadv/pwritev -- fbtl/posix takes its lseek+readv fallback.
 * The "absent" macros must stay UNDEFINED, not 0: several call sites
 * test them with '#if defined(...)'. */
#define HAVE_SEM_OPEN 1
#define HAVE_SEM_INIT 1
#define HAVE_LIBGEN_H 1
#define HAVE_SYS_PARAM_H 1
#define HAVE_SYS_STAT_H 1
#define HAVE_ALLOCA_H 1
#define HAVE_AIO_H 1
/* the win32 pthread shim implements process-shared mutexes/conds as
 * interlocked spin + generation counter valid across mapped segments */
#define HAVE_PTHREAD_MUTEXATTR_SETPSHARED 1
#define HAVE_PTHREAD_CONDATTR_SETPSHARED 1
/* #undef HAVE_AIO */
/* #undef HAVE_PREADV */
/* #undef HAVE_PWRITEV */
/* #undef HAVE_SYSLIMITS_H */
/* #undef HAVE_SYS_SYSCTL_H */
"""


def main():
    # args: mpi.h.in mpi.h.out major minor release mpi_ver mpi_subver
    #       [--defs-out fragment.h]
    # --defs-out emits every substituted '#define X v' (plus the internal
    # extras below) so the generated ompi_config.h can carry the same
    # config block that mpi.h's standalone section provides.
    defs_out = None
    if '--defs-out' in sys.argv:
        i = sys.argv.index('--defs-out')
        defs_out = sys.argv[i + 1]
        del sys.argv[i:i + 2]
    src, dst = sys.argv[1], sys.argv[2]
    vals = dict(DEFINES)
    vals['OMPI_MAJOR_VERSION'] = sys.argv[3]
    vals['OMPI_MINOR_VERSION'] = sys.argv[4]
    vals['OMPI_RELEASE_VERSION'] = sys.argv[5]
    vals['MPI_VERSION'] = sys.argv[6]
    vals['MPI_SUBVERSION'] = sys.argv[7]

    undef_re = re.compile(r'^#undef +([A-Za-z_][A-Za-z_0-9]*)\s*$')
    out = []
    substituted = set()
    for line in open(src, 'r', encoding='utf-8').read().splitlines():
        m = undef_re.match(line)
        if m:
            name = m.group(1)
            if name == 'ptrdiff_t':
                # autoconf quirk: emitted only when the type is missing;
                # MSVC/UCRT always has ptrdiff_t
                out.append('/* #undef ptrdiff_t */')
                continue
            if name in vals:
                out.append('#define %s %s' % (name, vals[name]))
                substituted.add(name)
                continue
            # unknown undefs: keep them commented rather than lose them
            out.append('/* #undef %s */' % name)
            continue
        out.append(line)

    missing = [k for k in vals if k not in substituted]
    if missing:
        sys.stderr.write('gen_mpi_h: no #undef found for: %s\n'
                         % ' '.join(sorted(missing)))

    with open(dst, 'w', encoding='utf-8', newline='\n') as f:
        f.write('\n'.join(out) + '\n')

    if defs_out:
        with open(defs_out, 'w', encoding='utf-8', newline='\n') as f:
            for k in sorted(vals):
                f.write('#define %s %s\n' % (k, vals[k]))
            # internal-only config macros (not in mpi.h's public section)
            f.write(INTERNAL_DEFINES)


if __name__ == '__main__':
    main()
