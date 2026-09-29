# Copyright (c) 2026      Amazon.com, Inc. or its affiliates.
#                         All Rights reserved.
# $COPYRIGHT$
#
# Additional copyrights may follow
#
# $HEADER$
# SPDX-License-Identifier: BSD-3-Clause-Open-MPI
#
# Native Windows (MSVC) CMake build for Open MPI.
#
# This file is the entry point for the Windows port.  It is NOT a
# replacement for the autotools build used on POSIX platforms; it only
# supports Windows/MSVC and a curated component set.

cmake_minimum_required(VERSION 3.24)

project(ompi_win32 C)

# ------------------------------------------------------------------
# project version (mirrors VERSION file parsing in configure.ac)
# ------------------------------------------------------------------
file(READ "${CMAKE_SOURCE_DIR}/VERSION" _version_text)
string(REGEX MATCH "major=([0-9]+)" _m "${_version_text}")
set(OMPI_MAJOR_VERSION "${CMAKE_MATCH_1}")
string(REGEX MATCH "minor=([0-9]+)" _m "${_version_text}")
set(OMPI_MINOR_VERSION "${CMAKE_MATCH_1}")
string(REGEX MATCH "release=([0-9]+)" _m "${_version_text}")
set(OMPI_RELEASE_VERSION "${CMAKE_MATCH_1}")
string(REGEX MATCH "greek=([^ \t\r\n]+)" _m "${_version_text}")
set(OMPI_GREEK_VERSION "${CMAKE_MATCH_1}")
set(OMPI_VERSION
    "${OMPI_MAJOR_VERSION}.${OMPI_MINOR_VERSION}.${OMPI_RELEASE_VERSION}${OMPI_GREEK_VERSION}")
set(OPAL_VERSION "${OMPI_VERSION}")
set(OPAL_SVN_REPO_REV "git")
set(OPAL_RELEASE_DATE "unreleased")
string(REGEX MATCH "mpi_standard_version=([0-9]+)" _m "${_version_text}")
set(OMPI_MPI_STANDARD_VERSION "${CMAKE_MATCH_1}")
string(REGEX MATCH "mpi_standard_subversion=([0-9]+)" _m "${_version_text}")
set(OMPI_MPI_STANDARD_SUBVERSION "${CMAKE_MATCH_1}")
set(OPAL_ARCH "x86_64-pc-windows-msvc")
if(DEFINED ENV{COMPUTERNAME})
    set(OPAL_CONFIGURE_HOST "$ENV{COMPUTERNAME}")
else()
    set(OPAL_CONFIGURE_HOST "windows-build")
endif()

set(PACKAGE_NAME "Open MPI")
set(PACKAGE_TARNAME "openmpi")
set(PACKAGE_VERSION "${OMPI_VERSION}")
set(PACKAGE_STRING "Open MPI ${OMPI_VERSION}")
set(PACKAGE_BUGREPORT "https://www.open-mpi.org/community/help/")
set(PACKAGE_URL "https://www.open-mpi.org/")

# ------------------------------------------------------------------
# toolchain requirements
# ------------------------------------------------------------------
if(NOT WIN32)
    message(FATAL_ERROR
        "This CMake build is Windows-only; use ./configure && make on POSIX")
endif()
if(NOT MSVC)
    message(FATAL_ERROR
        "The native Windows port requires MSVC (Visual Studio 2022)")
endif()

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)

find_package(Python3 REQUIRED COMPONENTS Interpreter)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
set(OMPI_SOURCE_DIR "${CMAKE_SOURCE_DIR}")

# ------------------------------------------------------------------
# locate the CRT/SDK include dirs for the POSIX forwarder headers
# ------------------------------------------------------------------
# MSVC tools dir: <VS>/VC/Tools/MSVC/<ver>/include
get_filename_component(_cl_dir "${CMAKE_C_COMPILER}" DIRECTORY)
# <cl> is at .../MSVC/<ver>/bin/Hostx64/x64/cl.exe -> strip 3 dirs
get_filename_component(_msvc_inc "${_cl_dir}/../../../include" ABSOLUTE)

# UCRT/UM/Shared dirs come from the Windows SDK.  CMAKE_VS_SDKS_* or the
# INCLUDE env var both work; simplest robust source is the INCLUDE env.
if(NOT DEFINED ENV{INCLUDE})
    message(FATAL_ERROR
        "INCLUDE env var not set; run cmake from a VS2022 developer prompt "
        "(vcvars64) or set CMAKE_SYSTEM_INCLUDE_PATH")
endif()
set(_ucrt_inc "")
set(_um_inc "")
set(_shared_inc "")
set(_inc_dirs "$ENV{INCLUDE}")
foreach(dir IN LISTS _inc_dirs)
    if(dir MATCHES "ucrt$" AND NOT _ucrt_inc)
        set(_ucrt_inc "${dir}")
    elseif(dir MATCHES "[\\/]um$" AND NOT _um_inc)
        set(_um_inc "${dir}")
    elseif(dir MATCHES "shared$" AND NOT _shared_inc)
        set(_shared_inc "${dir}")
    endif()
endforeach()
if(NOT _ucrt_inc OR NOT _um_inc OR NOT _shared_inc)
    message(FATAL_ERROR "could not locate Windows SDK include dirs")
endif()
message(STATUS "MSVC include:  ${_msvc_inc}")
message(STATUS "UCRT include:  ${_ucrt_inc}")
message(STATUS "UM include:    ${_um_inc}")
message(STATUS "Shared include: ${_shared_inc}")

# ------------------------------------------------------------------
# generate POSIX forwarder headers into <build>/win32fwd
# ------------------------------------------------------------------
set(OPAL_WIN32_FWD_DIR "${CMAKE_BINARY_DIR}/win32fwd")
include(OpalWin32Forwarders)
opal_win32_generate_forwarders("${OPAL_WIN32_FWD_DIR}"
    "${_msvc_inc}" "${_ucrt_inc}" "${_um_inc}" "${_shared_inc}")

# ------------------------------------------------------------------
# install directories (autotools-compatible layout under prefix)
# ------------------------------------------------------------------
include(GNUInstallDirs)
set(OPAL_BINDIR     "${CMAKE_INSTALL_BINDIR}")
set(OPAL_SBINDIR    "${CMAKE_INSTALL_SBINDIR}")
set(OPAL_LIBEXECDIR "${CMAKE_INSTALL_LIBEXECDIR}")
set(OPAL_DATAROOTDIR "${CMAKE_INSTALL_DATAROOTDIR}")
set(OPAL_DATADIR    "${CMAKE_INSTALL_DATADIR}")
set(OPAL_SYSCONFDIR "${CMAKE_INSTALL_SYSCONFDIR}")
set(OPAL_SHAREDSTATEDIR "${CMAKE_INSTALL_DATADIR}")
set(OPAL_LOCALSTATEDIR "${CMAKE_INSTALL_LOCALSTATEDIR}")
set(OPAL_LIBDIR     "${CMAKE_INSTALL_LIBDIR}")
set(OPAL_INCLUDEDIR "${CMAKE_INSTALL_INCLUDEDIR}")
set(OPAL_INFODIR    "${CMAKE_INSTALL_DATADIR}/info")
set(OPAL_MANDIR     "${CMAKE_INSTALL_MANDIR}")
set(OPAL_PKGDATADIR "${CMAKE_INSTALL_DATADIR}/openmpi")
set(OPAL_PKGLIBDIR  "${CMAKE_INSTALL_LIBDIR}/openmpi")
set(OPAL_PKGINCLUDEDIR "${CMAKE_INSTALL_INCLUDEDIR}/openmpi")
set(OPAL_DEFAULT_PKGUPDATEDIR "${CMAKE_INSTALL_LIBDIR}/openmpi")
set(OPAL_PARAM_DEFAULT_FILES_PREFIX
    "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_DATADIR}/openmpi")

# installdirs/config embeds the install tree at build time
# (install_dirs.h.in); emit absolute paths so no nested ${field}
# expansion is needed.
set(prefix "${CMAKE_INSTALL_PREFIX}")
set(exec_prefix "${CMAKE_INSTALL_PREFIX}")
set(bindir "${prefix}/${CMAKE_INSTALL_BINDIR}")
set(sbindir "${prefix}/${CMAKE_INSTALL_SBINDIR}")
set(libexecdir "${prefix}/${CMAKE_INSTALL_LIBEXECDIR}")
set(datarootdir "${prefix}/${CMAKE_INSTALL_DATAROOTDIR}")
set(datadir "${prefix}/${CMAKE_INSTALL_DATADIR}")
set(sysconfdir "${prefix}/${CMAKE_INSTALL_SYSCONFDIR}")
set(sharedstatedir "${prefix}/${CMAKE_INSTALL_SHAREDSTATEDIR}")
set(localstatedir "${prefix}/${CMAKE_INSTALL_LOCALSTATEDIR}")
set(libdir "${prefix}/${CMAKE_INSTALL_LIBDIR}")
set(includedir "${prefix}/${CMAKE_INSTALL_INCLUDEDIR}")
set(infodir "${prefix}/${CMAKE_INSTALL_INFODIR}")
set(mandir "${prefix}/${CMAKE_INSTALL_MANDIR}")
set(opaldatadir "${CMAKE_INSTALL_PREFIX}/${OPAL_PKGDATADIR}")
set(opallibdir "${CMAKE_INSTALL_PREFIX}/${OPAL_PKGLIBDIR}")
set(opalincludedir "${CMAKE_INSTALL_PREFIX}/${OPAL_PKGINCLUDEDIR}")
configure_file(opal/mca/installdirs/config/install_dirs.h.in
               "${CMAKE_BINARY_DIR}/opal/mca/installdirs/config/install_dirs.h"
               @ONLY)

# ------------------------------------------------------------------
# vendored third-party deps (headers now; compiled libraries in a
# later phase).  The vendored tarballs ship their own Windows config
# headers (hwloc/contrib/windows, libevent/WIN32-Code/nmake).
# ------------------------------------------------------------------
set(OPAL_DEPS_DIR "${CMAKE_BINARY_DIR}/deps")
set(_hwloc_src "${OPAL_DEPS_DIR}/hwloc-2.15.0")
set(_le_src    "${OPAL_DEPS_DIR}/libevent-2.1.12-stable-ompi")
if(NOT EXISTS "${_hwloc_src}/hwloc")
    message(STATUS "extracting vendored hwloc/libevent sources")
    file(MAKE_DIRECTORY "${OPAL_DEPS_DIR}")
    execute_process(COMMAND "${CMAKE_COMMAND}" -E tar xzf
        "${CMAKE_SOURCE_DIR}/3rd-party/hwloc-2.15.0.tar.gz"
        WORKING_DIRECTORY "${OPAL_DEPS_DIR}")
    execute_process(COMMAND "${CMAKE_COMMAND}" -E tar xzf
        "${CMAKE_SOURCE_DIR}/3rd-party/libevent-2.1.12-stable-ompi.tar.gz"
        WORKING_DIRECTORY "${OPAL_DEPS_DIR}")
endif()

# Build the vendored libraries with their own Windows CMake ports.
# IMPORTANT: add_subdirectory() BEFORE our global include_directories /
# add_compile_definitions below so the deps see a clean MSVC environment
# (no POSIX shadow headers, no OPAL_* defines).
set(EVENT__LIBRARY_TYPE "SHARED" CACHE STRING "" FORCE)
set(EVENT__DISABLE_TESTS    ON CACHE BOOL "" FORCE)
set(EVENT__DISABLE_BENCHMARK ON CACHE BOOL "" FORCE)
set(EVENT__DISABLE_REGRESS  ON CACHE BOOL "" FORCE)
set(EVENT__DISABLE_SAMPLES  ON CACHE BOOL "" FORCE)
set(EVENT__DISABLE_OPENSSL  ON CACHE BOOL "" FORCE)
add_subdirectory("${_le_src}" "${CMAKE_BINARY_DIR}/deps-build/libevent")

set(HWLOC_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
set(HWLOC_SKIP_TOOLS  ON  CACHE BOOL "" FORCE)
set(HWLOC_SKIP_LSTOPO ON  CACHE BOOL "" FORCE)
set(HWLOC_ENABLE_PLUGINS OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS ON  CACHE BOOL "" FORCE)
add_subdirectory("${_hwloc_src}/contrib/windows-cmake"
                 "${CMAKE_BINARY_DIR}/deps-build/hwloc")

# ------------------------------------------------------------------
# generated headers
# ------------------------------------------------------------------
file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/opal/include"
                    "${CMAKE_BINARY_DIR}/ompi/include")
# autotools generates opal/include/opal_config.h (AC_CONFIG_HEADERS);
# do the same so both "opal_config.h" and "opal/include/opal_config.h"
# include forms resolve.
configure_file(cmake/opal_config.h.cmake
               "${CMAKE_BINARY_DIR}/opal/include/opal_config.h" @ONLY)

# PMIx public headers: pmix_common.h/pmix_version.h are configure-time
# products of the embedded openpmix submodule.  Generate them now so
# opal/mca/pmix/base can compile; the library itself is Phase 4.
set(_pmix_inc "${CMAKE_BINARY_DIR}/pmix/include")
file(MAKE_DIRECTORY "${_pmix_inc}")
# read embedded PMIx version (3rd-party/openpmix/VERSION)
file(READ "${CMAKE_SOURCE_DIR}/3rd-party/openpmix/VERSION" _pv)
string(REGEX MATCH "major=([0-9]+)" _m "${_pv}")
set(pmixmajor "${CMAKE_MATCH_1}")
string(REGEX MATCH "minor=([0-9]+)" _m "${_pv}")
set(pmixminor "${CMAKE_MATCH_1}")
string(REGEX MATCH "release=([0-9]+)" _m "${_pv}")
set(pmixrelease "${CMAKE_MATCH_1}")
# config/pmix.m4: printf 0x%4.4x%2.2x%2.2x major minor release
math(EXPR _pmixnumeric
     "(${pmixmajor} << 16) + (${pmixminor} << 8) + ${pmixrelease}"
     OUTPUT_FORMAT HEXADECIMAL)
set(pmixnumeric "${_pmixnumeric}")
configure_file("${CMAKE_SOURCE_DIR}/3rd-party/openpmix/include/pmix_version.h.in"
               "${_pmix_inc}/pmix_version.h" @ONLY)
configure_file("${CMAKE_SOURCE_DIR}/3rd-party/openpmix/include/pmix_common.h.in"
               "${_pmix_inc}/pmix_common.h" COPYONLY)
# pmix_common.h.in leaves PMIX_EXPORT empty on non-visibility builds and
# has no Windows declspec path; the installed header must export from
# pmix.dll for builders and import for consumers.
file(READ "${_pmix_inc}/pmix_common.h" _pmix_common)
# autoheader would turn the bare "#undef" into a 0/1 define at
# configure time; COPYONLY leaves it literally undefining the macro,
# which also strips the pmix_config.h value from internal consumers.
string(REPLACE
"#undef PMIX_HAVE_VISIBILITY"
"#ifndef PMIX_HAVE_VISIBILITY
#    define PMIX_HAVE_VISIBILITY 0
#endif" _pmix_common "${_pmix_common}")
string(REPLACE
"#if PMIX_HAVE_VISIBILITY == 1
#define PMIX_EXPORT __attribute__((__visibility__(\"default\")))
#else
#define PMIX_EXPORT
#endif"
"#if defined(_MSC_VER)
#    if defined(PMIX_BUILDING_DLL) && PMIX_BUILDING_DLL
#        define PMIX_EXPORT __declspec(dllexport)
#    else
#        define PMIX_EXPORT __declspec(dllimport)
#    endif
#elif PMIX_HAVE_VISIBILITY == 1
#    define PMIX_EXPORT __attribute__((__visibility__(\"default\")))
#else
#    define PMIX_EXPORT
#endif" _pmix_common "${_pmix_common}")
file(WRITE "${_pmix_inc}/pmix_common.h" "${_pmix_common}")

# pmix_frameworks.h is generated by openpmix's autogen.pl listing its
# MCA frameworks; the real registry (with pmix_frameworks.c) is written
# by the PMIx section below.

# pmix_config.h -- PMIx's internal config header; generated from
# cmake/pmix_config.h.cmake into pmix_gen/src/include/ in the PMIx
# section below.  OPAL's mca/pmix shim headers also resolve it via the
# global include path (pmix_gen precedes the openpmix source root).

# stub ompi_config.h -- replaced by the real OMPI configuration when
# the ompi/ subtree is enabled.  A handful of OPAL sources include it
# unconditionally in a combined build (e.g. smsc/base/base.h).
# NOTE: always regenerated -- a previous configure's copy may be stale.
# ompi_config.h at the build root -- OPAL-layer sources (e.g.
# smsc/base/base.h) include "ompi_config.h" without the ompi/include
# prefix.  Forward to the real generated header (written below at
# ompi/include/ompi_config.h).
file(WRITE "${CMAKE_BINARY_DIR}/ompi_config.h"
"/* forwarding header for the Windows CMake build */
#include \"ompi/include/ompi_config.h\"
")

# ------------------------------------------------------------------
# common compile settings for all in-tree targets
# ------------------------------------------------------------------
add_compile_definitions(
    _CRT_SECURE_NO_WARNINGS=1
    _CRT_NON_CONFORMING_SWPRINTFS=1
    WIN32_LEAN_AND_MEAN
    NOMINMAX
    _WINSOCK_DEPRECATED_NO_WARNINGS
    _WIN32_WINNT=0x0A00
)
add_compile_options(
    /wd4100   # unreferenced parameter (POSIX stub args)
    /wd4115   # named type in parens
    /wd4127   # conditional constant
    /wd4142   # benign typedef redefinition
    /wd4244   # conversion warnings in legacy code paths
    /wd4267   # size_t -> int conversions
    /wd4310   # cast truncates constant
    /wd4389   # signed/unsigned compare
    /wd4706   # assignment in conditional
    /wd4996   # deprecated CRT functions (compat layer wraps them)
    /Zc:preprocessor
    /wd4068   # unknown pragma
    /experimental:c11atomics   # PMIx requires C11 <stdatomic.h>; the flag
                             # also unlocks _Atomic for any in-tree use
)

# include search order: build-dir generated -> opal shadow -> forwarders
# -> source-tree public headers
include_directories(
    "${CMAKE_BINARY_DIR}"
    "${CMAKE_BINARY_DIR}/opal/include"
    "${CMAKE_SOURCE_DIR}/opal/win32/include"
    "${OPAL_WIN32_FWD_DIR}"
    "${CMAKE_SOURCE_DIR}/opal/include"
    "${CMAKE_SOURCE_DIR}/3rd-party/openpmix/include"
    "${CMAKE_BINARY_DIR}/pmix/include"
    "${CMAKE_BINARY_DIR}/pmix_gen"
    "${CMAKE_SOURCE_DIR}/3rd-party/openpmix"
)

# ------------------------------------------------------------------
# PMIx -- real openpmix build (Phase 4)
#
# Compiles the embedded 3rd-party/openpmix sources into pmix.dll.
# POSIX portability is provided by the opal/win32 compatibility layer:
# the shadow headers map POSIX names onto opal_win32_* symbols, and the
# implementations are imported from open-pal.dll.  Windows-appropriate
# HAVE_* answers live in cmake/pmix_config.h.cmake.
# ------------------------------------------------------------------
set(PMIX_SOURCE_DIR "${CMAKE_SOURCE_DIR}/3rd-party/openpmix")
set(PMIX_GEN_DIR "${CMAKE_BINARY_DIR}/pmix_gen")

# pmix version (parsed from its VERSION file)
file(STRINGS "${PMIX_SOURCE_DIR}/VERSION" _pmix_vlines
     REGEX "^(major|minor|release|greek|repo_rev|date|tarball_version|std_abi_stable|std_abi_provisional|std_major|std_minor|std_extension)=")
foreach(_vl IN LISTS _pmix_vlines)
    string(REGEX MATCH "^([a-z_]+)=(.*)$" _m "${_vl}")
    set(pmix_${CMAKE_MATCH_1} "${CMAKE_MATCH_2}")
endforeach()
string(REGEX REPLACE "\"" "" pmix_date "${pmix_date}")
set(pmixversion "${pmix_major}.${pmix_minor}.${pmix_release}${pmix_greek}")
set(pmixmajor "${pmix_major}")
set(pmixminor "${pmix_minor}")
set(pmixrelease "${pmix_release}")
set(pmixgreek "${pmix_greek}")
set(pmixreporev "${pmix_repo_rev}")
set(pmixreleasedate "${pmix_date}")

# provenance + Standard ABI levels (VERSION's std_abi_* lines)
set(pmix_std_abi_stable "${pmix_std_abi_stable}")
set(pmix_std_abi_provisional "${pmix_std_abi_provisional}")
set(pmix_std_version "${pmix_std_major}.${pmix_std_minor}.${pmix_std_extension}")
string(TIMESTAMP _pmix_build_date "%Y-%m-%d" UTC)
cmake_host_system_information(RESULT _pmix_build_host QUERY HOSTNAME)
if(DEFINED ENV{USERNAME})
    set(_pmix_build_user "$ENV{USERNAME}")
else()
    set(_pmix_build_user "unknown")
endif()

# pmix install dirs (absolute build-tree paths)
set(PMIX_BINDIR "${CMAKE_BINARY_DIR}/bin")
set(PMIX_SBINDIR "${CMAKE_BINARY_DIR}/sbin")
set(PMIX_LIBEXECDIR "${CMAKE_BINARY_DIR}/libexec")
set(PMIX_DATAROOTDIR "${CMAKE_BINARY_DIR}/share")
set(PMIX_DATADIR "${CMAKE_BINARY_DIR}/share")
set(PMIX_SYSCONFDIR "${CMAKE_BINARY_DIR}/etc")
set(PMIX_SHAREDSTATEDIR "${CMAKE_BINARY_DIR}/share")
set(PMIX_LOCALSTATEDIR "${CMAKE_BINARY_DIR}/var")
set(PMIX_LIBDIR "${CMAKE_BINARY_DIR}/lib")
set(PMIX_INCLUDEDIR "${CMAKE_BINARY_DIR}/include")
set(PMIX_INFODIR "${CMAKE_BINARY_DIR}/share/info")
set(PMIX_MANDIR "${CMAKE_BINARY_DIR}/share/man")
set(PMIX_PKGDATADIR "${CMAKE_BINARY_DIR}/share/openpmix")
set(PMIX_PKGLIBDIR "${CMAKE_BINARY_DIR}/lib")
set(PMIX_PKGINCLUDEDIR "${CMAKE_BINARY_DIR}/include")

configure_file("${CMAKE_SOURCE_DIR}/cmake/pmix_config.h.cmake"
               "${PMIX_GEN_DIR}/src/include/pmix_config.h" @ONLY)

# pinstall_dirs.h -- the pinstalldirs/config component's compiled-in
# install paths (GNU dir-var @tokens@)
set(prefix "${CMAKE_BINARY_DIR}")
set(exec_prefix "${CMAKE_BINARY_DIR}")
set(bindir "${PMIX_BINDIR}")
set(sbindir "${PMIX_SBINDIR}")
set(libexecdir "${PMIX_LIBEXECDIR}")
set(datarootdir "${PMIX_DATAROOTDIR}")
set(datadir "${PMIX_DATADIR}")
set(pmixdatadir "${PMIX_PKGDATADIR}")
set(sysconfdir "${PMIX_SYSCONFDIR}")
set(sharedstatedir "${PMIX_SHAREDSTATEDIR}")
set(localstatedir "${PMIX_LOCALSTATEDIR}")
set(libdir "${PMIX_LIBDIR}")
set(pmixlibdir "${PMIX_PKGLIBDIR}")
set(includedir "${PMIX_INCLUDEDIR}")
set(pmixincludedir "${PMIX_PKGINCLUDEDIR}")
set(infodir "${PMIX_INFODIR}")
set(mandir "${PMIX_MANDIR}")
configure_file(
    "${PMIX_SOURCE_DIR}/src/mca/pinstalldirs/config/pinstall_dirs.h.in"
    "${PMIX_GEN_DIR}/src/mca/pinstalldirs/config/pinstall_dirs.h" @ONLY)

# generated sources: pmix_dictionary.{c,h}, pmix_event_strings.{c,h}
file(MAKE_DIRECTORY "${PMIX_GEN_DIR}/src/include")
execute_process(
    COMMAND "${Python3_EXECUTABLE}"
            "${PMIX_SOURCE_DIR}/contrib/construct_dictionary.py"
            --root "${PMIX_SOURCE_DIR}"
            --out "${PMIX_GEN_DIR}/src/include"
    RESULT_VARIABLE _pmd_rc
    ERROR_VARIABLE _pmd_err)
if(NOT _pmd_rc EQUAL 0)
    message(FATAL_ERROR "construct_dictionary.py failed: ${_pmd_err}")
endif()
execute_process(
    COMMAND "${Python3_EXECUTABLE}"
            "${PMIX_SOURCE_DIR}/contrib/construct_event_strings.py"
    WORKING_DIRECTORY "${PMIX_GEN_DIR}/src/include"
    RESULT_VARIABLE _pes_rc
    ERROR_VARIABLE _pes_err)
if(NOT _pes_rc EQUAL 0)
    message(FATAL_ERROR "construct_event_strings.py failed: ${_pes_err}")
endif()

# enabled frameworks and their static components.  Each component dir is
# globbed for *.c; the framework base dirs are globbed likewise.
set(PMIX_FRAMEWORK_LIST
    bfrops gds pcompress pdl pgpu pif pinstalldirs plog pmdl pnet preg
    psec psensor pstat ptl)
set(PMIX_COMPONENT_LIST
    bfrops/v21 bfrops/v3 bfrops/v4 bfrops/v41 bfrops/v51 bfrops/v61
    gds/hash gds/shmem3
    pdl/pdlopen
    pif/bsdx_ipv4
    pinstalldirs/env pinstalldirs/config
    plog/stdfd
    pmdl/ompi
    preg/raw preg/compress
    psec/native psec/none
    ptl/client ptl/server ptl/tool)

# per-framework static-components.h (generated into pmix_gen tree, which
# precedes the real source root on the include path)
set(_fw_with_components "")
foreach(_fw IN LISTS PMIX_FRAMEWORK_LIST)
    set(_comps "")
    foreach(_cp IN LISTS PMIX_COMPONENT_LIST)
        if(_cp MATCHES "^${_fw}/")
            string(REGEX REPLACE "^${_fw}/" "" _cn "${_cp}")
            list(APPEND _comps "${_cn}")
        endif()
    endforeach()
    set(_sc "/* generated by CMake -- PMIx static component list */\n#if defined(c_plusplus) || defined(__cplusplus)\nextern \"C\" {\n#endif\n")
    set(_arr "")
    foreach(_cn IN LISTS _comps)
        string(REPLACE "-" "_" _cns "${_cn}")
        set(_sc "${_sc}extern const pmix_mca_base_component_t *pmix_mca_${_fw}_${_cns}_component_ptr;\n")
        set(_arr "${_arr}    &pmix_mca_${_fw}_${_cns}_component_ptr,\n")
    endforeach()
    set(_sc "${_sc}#if defined(c_plusplus) || defined(__cplusplus)\n}\n#endif\n\nconst pmix_mca_base_component_t **pmix_mca_${_fw}_base_static_components[] = {\n${_arr}    NULL\n};\n")
    file(MAKE_DIRECTORY "${PMIX_GEN_DIR}/src/mca/${_fw}/base")
    file(WRITE "${PMIX_GEN_DIR}/src/mca/${_fw}/base/static-components.h" "${_sc}")
    if(_comps)
        list(APPEND _fw_with_components "${_fw}")
    endif()
endforeach()

# pmix_frameworks.h/.c -- the project framework registry
set(_pfh "/* generated by CMake -- PMIx framework registry */\n#ifndef PMIX_FRAMEWORKS_H\n#define PMIX_FRAMEWORKS_H\n\n#include \"src/mca/base/pmix_mca_base_framework.h\"\n\n")
set(_pfc "/* generated by CMake -- PMIx framework registry */\n#include \"src/include/pmix_config.h\"\n#include \"src/mca/mca.h\"\n#include \"src/include/pmix_frameworks.h\"\n\n")
foreach(_fw IN LISTS PMIX_FRAMEWORK_LIST)
    set(_pfh "${_pfh}PMIX_EXPORT extern pmix_mca_base_framework_t pmix_${_fw}_base_framework;\n")
    set(_pfc "${_pfc}extern pmix_mca_base_framework_t pmix_${_fw}_base_framework;\n")
endforeach()
set(_pfh "${_pfh}\nPMIX_EXPORT extern pmix_mca_base_framework_t *pmix_frameworks[];\nPMIX_EXPORT extern char *pmix_framework_names[];\n\n#endif\n")
set(_pfc "${_pfc}\npmix_mca_base_framework_t *pmix_frameworks[] = {\n")
set(_pfnames "")
foreach(_fw IN LISTS PMIX_FRAMEWORK_LIST)
    set(_pfc "${_pfc}    &pmix_${_fw}_base_framework,\n")
    set(_pfnames "${_pfnames}    \"${_fw}\",\n")
endforeach()
set(_pfc "${_pfc}    NULL\n};\n\nchar *pmix_framework_names[] = {\n${_pfnames}    NULL\n};\n")
file(WRITE "${PMIX_GEN_DIR}/src/include/pmix_frameworks.h" "${_pfh}")
file(WRITE "${PMIX_GEN_DIR}/src/include/pmix_frameworks.c" "${_pfc}")

# ---- source inventory ---------------------------------------------
# core dirs: glob *.c (pmix keeps its file lists in Makefile.includes;
# globbing is equivalent for our single-platform bring-up)
# pmix_show_help_content.c embeds every help-*.txt in the tree (same
# generator mechanism as opal's convert-help-files-to-c-code.py)
set(PMIX_SHOW_HELP_CONTENT "${PMIX_GEN_DIR}/pmix_show_help_content.c")
file(MAKE_DIRECTORY "${PMIX_GEN_DIR}")
execute_process(
    COMMAND "${Python3_EXECUTABLE}"
            "${PMIX_SOURCE_DIR}/src/util/convert-help.py"
            --root "${PMIX_SOURCE_DIR}"
            --out "${PMIX_SHOW_HELP_CONTENT}"
    RESULT_VARIABLE _psh_rc
    ERROR_VARIABLE _psh_err)
if(NOT _psh_rc EQUAL 0)
    message(FATAL_ERROR "convert-help.py failed: ${_psh_err}")
endif()

set(PMIX_SOURCES "")
foreach(_d class threads event hwloc runtime client server tool common util include mca/base)
    file(GLOB _srcs "${PMIX_SOURCE_DIR}/src/${_d}/*.c")
    list(APPEND PMIX_SOURCES ${_srcs})
endforeach()
# framework bases + components
foreach(_fw IN LISTS PMIX_FRAMEWORK_LIST)
    file(GLOB _srcs "${PMIX_SOURCE_DIR}/src/mca/${_fw}/base/*.c")
    list(APPEND PMIX_SOURCES ${_srcs})
endforeach()
foreach(_cp IN LISTS PMIX_COMPONENT_LIST)
    file(GLOB _srcs "${PMIX_SOURCE_DIR}/src/mca/${_cp}/*.c")
    list(APPEND PMIX_SOURCES ${_srcs})
endforeach()
list(APPEND PMIX_SOURCES
    "${PMIX_SHOW_HELP_CONTENT}"
    "${PMIX_GEN_DIR}/src/include/pmix_dictionary.c"
    "${PMIX_GEN_DIR}/src/include/pmix_event_strings.c"
    "${PMIX_GEN_DIR}/src/include/pmix_frameworks.c")

# tools/examples/tests never belong to the library
list(FILTER PMIX_SOURCES EXCLUDE REGEX "/tools/|/test/|/examples/")

# Private copy of the opal/win32 POSIX compat implementations for
# pmix.dll.  open-pal.dll has its own (exported) copies; linking open-pal
# here instead would create a shared-library dependency cycle (opal's
# mca/pmix shim calls PMIx_* APIs).  OPAL_WIN32_STATIC gives both sides
# plain extern decls so the static objects link into pmix.dll directly.
file(GLOB PMIX_WIN32COMPAT_SOURCES "${CMAKE_SOURCE_DIR}/opal/win32/*.c")
add_library(pmix_win32compat STATIC ${PMIX_WIN32COMPAT_SOURCES})
# OPAL_WIN32_STATIC -> plain OPAL_WIN32_DECLSPEC decls/defs (they link
# into pmix.dll); OPAL_BUILDING -> OPAL_DECLSPEC decls in a few impls
# that share headers with OPAL (e.g. opal_socket_errno.h) stay valid.
target_compile_definitions(pmix_win32compat PUBLIC OPAL_WIN32_STATIC=1)
target_compile_definitions(pmix_win32compat PRIVATE OPAL_BUILDING=1)
target_include_directories(pmix_win32compat PUBLIC
    "${CMAKE_SOURCE_DIR}/opal/win32/include"
    "${OPAL_WIN32_FWD_DIR}"
    "${CMAKE_SOURCE_DIR}"
    "${CMAKE_BINARY_DIR}/opal/include"
    "${CMAKE_SOURCE_DIR}/opal/include")

add_library(pmix SHARED ${PMIX_SOURCES})
# PMIX_BUILDING gates visibility of PMIx internals (pmix_config_bottom.h
# defaults it to 1 for consumers of internal headers); PMIX_BUILDING_DLL
# drives the dllexport polarity so internal-header consumers like PRRTE
# still get dllimport
target_compile_definitions(pmix PRIVATE PMIX_BUILDING=1 PMIX_BUILDING_DLL=1)
# MSVC keeps <stdatomic.h> behind an experimental gate even in C11 mode
target_compile_options(pmix PRIVATE /experimental:c11atomics)
target_include_directories(pmix PRIVATE
    "${PMIX_GEN_DIR}"
    "${PMIX_GEN_DIR}/src/include"
    "${CMAKE_BINARY_DIR}/pmix"
    "${PMIX_SOURCE_DIR}"
    "${PMIX_SOURCE_DIR}/src"
    "${PMIX_SOURCE_DIR}/include"
    "${CMAKE_BINARY_DIR}/pmix/include"
    "${CMAKE_SOURCE_DIR}/opal/win32/include"
    "${OPAL_WIN32_FWD_DIR}"
    "${CMAKE_BINARY_DIR}/deps/le-include"
    "${CMAKE_BINARY_DIR}/deps-build/libevent/include"
    "${_le_src}/include"
    "${CMAKE_BINARY_DIR}/deps-build/hwloc/include"
    "${_hwloc_src}/include"
)
target_link_libraries(pmix PRIVATE
    pmix_win32compat
    event event_core
    hwloc
    ws2_32 iphlpapi advapi32 dbghelp shlwapi)
set_target_properties(pmix PROPERTIES
    C_STANDARD 11
    C_EXTENSIONS OFF
    OUTPUT_NAME pmix
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")

# ------------------------------------------------------------------
# PRRTE -- real prrte build (Phase 7)
# Compiles the embedded 3rd-party/prrte sources into prte.dll plus the
# launcher tools (prte/prterun/mpirun, prted, prte-submit, pterm,
# prte_info).  Same generated-header + static-component strategy as the
# PMIx block above; HAVE_* answers live in cmake/prte_config.h.cmake.
# ------------------------------------------------------------------
set(PRTE_SOURCE_DIR "${CMAKE_SOURCE_DIR}/3rd-party/prrte")
set(PRTE_GEN_DIR "${CMAKE_BINARY_DIR}/prte_gen")

# prte version (parsed from its VERSION file)
file(STRINGS "${PRTE_SOURCE_DIR}/VERSION" _prte_vlines
     REGEX "^[a-z_]+=")
foreach(_vl IN LISTS _prte_vlines)
    if(_vl MATCHES "^([a-z_]+)=(.*)$")
        set(prte_${CMAKE_MATCH_1} "${CMAKE_MATCH_2}")
    endif()
endforeach()
string(REGEX REPLACE "\"" "" prte_greek "${prte_greek}")
string(REGEX REPLACE "\"" "" prte_date "${prte_date}")
set(prteversion "${prte_major}.${prte_minor}.${prte_release}${prte_greek}")
set(prtemajor "${prte_major}")
set(prteminor "${prte_minor}")
set(prterelease "${prte_release}")
set(prtegreek "${prte_greek}")
set(prtereporev "gitclone")
set(prterelease_date "${prte_date}")
string(TIMESTAMP _prte_build_date "%Y-%m-%d" UTC)
cmake_host_system_information(RESULT _prte_build_host QUERY HOSTNAME)
if(DEFINED ENV{USERNAME})
    set(_prte_build_user "$ENV{USERNAME}")
else()
    set(_prte_build_user "unknown")
endif()

# install dirs (absolute build-tree paths)
set(PRTE_BINDIR "${CMAKE_BINARY_DIR}/bin")
set(PRTE_SBINDIR "${CMAKE_BINARY_DIR}/sbin")
set(PRTE_LIBEXECDIR "${CMAKE_BINARY_DIR}/libexec")
set(PRTE_DATAROOTDIR "${CMAKE_BINARY_DIR}/share")
set(PRTE_DATADIR "${CMAKE_BINARY_DIR}/share")
set(PRTE_SYSCONFDIR "${CMAKE_BINARY_DIR}/etc")
set(PRTE_SHAREDSTATEDIR "${CMAKE_BINARY_DIR}/share")
set(PRTE_LOCALSTATEDIR "${CMAKE_BINARY_DIR}/var")
set(PRTE_LIBDIR "${CMAKE_BINARY_DIR}/lib")
set(PRTE_INCLUDEDIR "${CMAKE_BINARY_DIR}/include")
set(PRTE_INFODIR "${CMAKE_BINARY_DIR}/share/info")
set(PRTE_MANDIR "${CMAKE_BINARY_DIR}/share/man")
set(PRTE_PKGDATADIR "${CMAKE_BINARY_DIR}/share/prte")
set(PRTE_PKGLIBDIR "${CMAKE_BINARY_DIR}/lib")
set(PRTE_PKGINCLUDEDIR "${CMAKE_BINARY_DIR}/include")

file(MAKE_DIRECTORY "${PRTE_GEN_DIR}/src/include")
configure_file("${CMAKE_SOURCE_DIR}/cmake/prte_config.h.cmake"
               "${PRTE_GEN_DIR}/prte_config.h" @ONLY)
# the generated show-help source includes it as "src/include/prte_config.h"
configure_file("${CMAKE_SOURCE_DIR}/cmake/prte_config.h.cmake"
               "${PRTE_GEN_DIR}/src/include/prte_config.h" @ONLY)

# version.h from version.h.in
set(PRTE_VERSION "${prteversion}")
set(PRTE_MAJOR_VERSION "${prte_major}")
set(PRTE_MINOR_VERSION "${prte_minor}")
set(PRTE_GREEK_VERSION "${prte_greek}")
set(PRTE_WANT_REPO_REV 0)
set(PRTE_REPO_REV "gitclone")
configure_file("${PRTE_SOURCE_DIR}/src/include/version.h.in"
               "${PRTE_GEN_DIR}/src/include/version.h" @ONLY)

# include/prte_version.h (numeric version, consumed by include/prte.h)
math(EXPR _prtenumeric
     "(${prtemajor} << 16) + (${prteminor} << 8) + ${prterelease}"
     OUTPUT_FORMAT HEXADECIMAL)
set(prtenumeric "0x${_prtenumeric}")
set(prtemajor_v "${prtemajor}")
set(prteminor_v "${prteminor}")
set(prterelease_v "${prterelease}")
configure_file("${PRTE_SOURCE_DIR}/include/prte_version.h.in"
               "${PRTE_GEN_DIR}/include/prte_version.h" @ONLY)

# install_dirs.h -- prteinstalldirs/config component's compiled-in paths
set(prefix "${CMAKE_BINARY_DIR}")
set(exec_prefix "${CMAKE_BINARY_DIR}")
set(bindir "${PRTE_BINDIR}")
set(sbindir "${PRTE_SBINDIR}")
set(libexecdir "${PRTE_LIBEXECDIR}")
set(datarootdir "${PRTE_DATAROOTDIR}")
set(datadir "${PRTE_DATADIR}")
set(prtedatadir "${PRTE_PKGDATADIR}")
set(sysconfdir "${PRTE_SYSCONFDIR}")
set(sharedstatedir "${PRTE_SHAREDSTATEDIR}")
set(localstatedir "${PRTE_LOCALSTATEDIR}")
set(libdir "${PRTE_LIBDIR}")
set(prtelibdir "${PRTE_PKGLIBDIR}")
set(includedir "${PRTE_INCLUDEDIR}")
set(prteincludedir "${PRTE_PKGINCLUDEDIR}")
set(infodir "${PRTE_INFODIR}")
set(mandir "${PRTE_MANDIR}")
file(MAKE_DIRECTORY "${PRTE_GEN_DIR}/src/mca/prteinstalldirs/config")
configure_file(
    "${PRTE_SOURCE_DIR}/src/mca/prteinstalldirs/config/install_dirs.h.in"
    "${PRTE_GEN_DIR}/src/mca/prteinstalldirs/config/install_dirs.h" @ONLY)

# frameworks and their static components.  Minimal set for a working
# DVM on a single Windows host: env+hnp ess, ssh plm (local spawn path),
# pmix+hosts ras, the portable rmaps policies, pdefault odls, hnp+prted
# iof/errmgr/state, ompi+prte schizo, raw filem, weighted reachable,
# env+config installdirs, none backtrace.
set(PRTE_FRAMEWORK_LIST
    errmgr ess filem iof odls plm prtebacktrace prteinstalldirs
    prtereachable ras rmaps schizo state)
set(PRTE_COMPONENT_LIST
    ess/env ess/hnp
    plm/ssh
    ras/pmix ras/hosts
    rmaps/ppr rmaps/rank_file rmaps/round_robin rmaps/seq
    odls/pdefault
    iof/hnp iof/prted
    errmgr/dvm errmgr/prted
    state/dvm state/prted
    schizo/ompi schizo/prte
    filem/raw
    prtereachable/weighted
    prteinstalldirs/env prteinstalldirs/config
    prtebacktrace/none)

# per-framework static-components.h
set(_prte_fw_with_components "")
foreach(_fw IN LISTS PRTE_FRAMEWORK_LIST)
    set(_comps "")
    foreach(_cp IN LISTS PRTE_COMPONENT_LIST)
        if(_cp MATCHES "^${_fw}/")
            string(REGEX REPLACE "^${_fw}/" "" _cn "${_cp}")
            list(APPEND _comps "${_cn}")
        endif()
    endforeach()
    set(_sc "/* generated by CMake -- PRRTE static component list */\n#if defined(c_plusplus) || defined(__cplusplus)\nextern \"C\" {\n#endif\n")
    set(_arr "")
    foreach(_cn IN LISTS _comps)
        string(REPLACE "-" "_" _cns "${_cn}")
        set(_sc "${_sc}extern const pmix_mca_base_component_t *prte_mca_${_fw}_${_cns}_component_ptr;\n")
        set(_arr "${_arr}    &prte_mca_${_fw}_${_cns}_component_ptr,\n")
    endforeach()
    set(_sc "${_sc}#if defined(c_plusplus) || defined(__cplusplus)\n}\n#endif\n\nconst pmix_mca_base_component_t **prte_${_fw}_base_static_components[] = {\n${_arr}    NULL\n};\n")
    file(MAKE_DIRECTORY "${PRTE_GEN_DIR}/src/mca/${_fw}/base")
    file(WRITE "${PRTE_GEN_DIR}/src/mca/${_fw}/base/static-components.h" "${_sc}")
    if(_comps)
        list(APPEND _prte_fw_with_components "${_fw}")
    endif()
endforeach()

# prte_frameworks.h/.c -- the project framework registry (consumed via
# "src/include/prte_frameworks.h" in src/runtime/prte_init.c)
set(_pfh "/* generated by CMake -- PRRTE framework registry */\n#ifndef PRTE_FRAMEWORKS_H\n#define PRTE_FRAMEWORKS_H\n\n#include \"src/mca/base/pmix_mca_base_framework.h\"\n\n")
set(_pfc "/* generated by CMake -- PRRTE framework registry */\n#include \"src/include/prte_config.h\"\n#include \"src/include/prte_frameworks.h\"\n\n")
foreach(_fw IN LISTS PRTE_FRAMEWORK_LIST)
    set(_pfh "${_pfh}PRTE_EXPORT extern pmix_mca_base_framework_t prte_${_fw}_base_framework;\n")
    set(_pfc "${_pfc}extern pmix_mca_base_framework_t prte_${_fw}_base_framework;\n")
endforeach()
set(_pfh "${_pfh}\nPRTE_EXPORT extern pmix_mca_base_framework_t *prte_frameworks[];\nPRTE_EXPORT extern char *prte_framework_names[];\n\n#endif\n")
set(_pfc "${_pfc}\npmix_mca_base_framework_t *prte_frameworks[] = {\n")
set(_pfnames "")
foreach(_fw IN LISTS PRTE_FRAMEWORK_LIST)
    set(_pfc "${_pfc}    &prte_${_fw}_base_framework,\n")
    set(_pfnames "${_pfnames}    \"${_fw}\",\n")
endforeach()
set(_pfc "${_pfc}    NULL\n};\n\nchar *prte_framework_names[] = {\n${_pfnames}    NULL\n};\n")
file(WRITE "${PRTE_GEN_DIR}/src/include/prte_frameworks.h" "${_pfh}")
file(WRITE "${PRTE_GEN_DIR}/src/include/prte_frameworks.c" "${_pfc}")

# generated show-help content (all help-*.txt in the prrte tree)
set(PRTE_SHOW_HELP_CONTENT "${PRTE_GEN_DIR}/prte_show_help_content.c")
execute_process(
    COMMAND "${CMAKE_COMMAND}" -E env "PYTHONUTF8=1" "${Python3_EXECUTABLE}"
            "${PRTE_SOURCE_DIR}/src/util/prte-convert-help.py"
            --root "${PRTE_SOURCE_DIR}"
            --out "${PRTE_SHOW_HELP_CONTENT}"
    RESULT_VARIABLE _psh_rc
    ERROR_VARIABLE _psh_err)
if(NOT _psh_rc EQUAL 0)
    message(FATAL_ERROR "prte-convert-help.py failed: ${_psh_err}")
endif()

# ---- source inventory ---------------------------------------------
set(PRTE_SOURCES "")
foreach(_d event grpcomm hwloc pmix runtime util
          runtime/data_server runtime/data_type_support
          util/dash_host util/hostfile util/rankfile
          rml rml/oob rml/relm prted prted/pmix)
    file(GLOB _srcs "${PRTE_SOURCE_DIR}/src/${_d}/*.c")
    list(APPEND PRTE_SOURCES ${_srcs})
endforeach()
# framework bases + components
foreach(_fw IN LISTS PRTE_FRAMEWORK_LIST)
    file(GLOB _srcs "${PRTE_SOURCE_DIR}/src/mca/${_fw}/base/*.c")
    list(APPEND PRTE_SOURCES ${_srcs})
endforeach()
foreach(_cp IN LISTS PRTE_COMPONENT_LIST)
    file(GLOB _srcs "${PRTE_SOURCE_DIR}/src/mca/${_cp}/*.c")
    list(APPEND PRTE_SOURCES ${_srcs})
endforeach()
list(APPEND PRTE_SOURCES
    "${PRTE_SHOW_HELP_CONTENT}"
    "${PRTE_GEN_DIR}/src/include/prte_frameworks.c")

# the generated show-help content is UTF-8 (help files contain non-ASCII
# text); MSVC otherwise reads it in the system codepage and can split
# multibyte sequences inside string literals
set_source_files_properties("${PRTE_SHOW_HELP_CONTENT}"
    PROPERTIES COMPILE_OPTIONS "/utf-8")

# component-local -D flags carried by the upstream Makefile.am files
if(EXISTS "${PRTE_SOURCE_DIR}/src/mca/plm/ssh/plm_ssh_component.c")
    set_source_files_properties("${PRTE_SOURCE_DIR}/src/mca/plm/ssh/plm_ssh_component.c"
        PROPERTIES COMPILE_DEFINITIONS "PRTE_PBSTRMSH_PATH=\"\"")
endif()
if(EXISTS "${PRTE_SOURCE_DIR}/src/mca/schizo/ompi/schizo_ompi.c")
    set_source_files_properties("${PRTE_SOURCE_DIR}/src/mca/schizo/ompi/schizo_ompi.c"
        PROPERTIES COMPILE_DEFINITIONS "DEFAULT_PARAM_FILE_PATH=\"${CMAKE_BINARY_DIR}/share/openmpi\"")
endif()

# tools are separate executables, not part of the library
list(FILTER PRTE_SOURCES EXCLUDE REGEX "/tools/")

add_library(prrte SHARED ${PRTE_SOURCES})
target_compile_definitions(prrte PRIVATE PRTE_BUILDING=1)
target_compile_options(prrte PRIVATE /experimental:c11atomics)
target_include_directories(prrte PRIVATE
    "${PRTE_GEN_DIR}"
    "${PRTE_GEN_DIR}/include"
    "${PRTE_GEN_DIR}/src/include"
    "${PRTE_SOURCE_DIR}"
    "${PRTE_SOURCE_DIR}/src"
    "${PRTE_SOURCE_DIR}/src/include"
    "${PMIX_GEN_DIR}"
    "${PMIX_GEN_DIR}/src/include"
    "${PMIX_SOURCE_DIR}"
    "${PMIX_SOURCE_DIR}/src"
    "${PMIX_SOURCE_DIR}/include"
    "${CMAKE_BINARY_DIR}/pmix/include"
    "${CMAKE_BINARY_DIR}/pmix"
    "${CMAKE_SOURCE_DIR}/opal/win32/include"
    "${OPAL_WIN32_FWD_DIR}"
    "${CMAKE_BINARY_DIR}/deps-build/libevent/include"
    "${_le_src}/include"
    "${CMAKE_BINARY_DIR}/deps-build/hwloc/include"
    "${_hwloc_src}/include")
target_link_libraries(prrte PRIVATE
    pmix_win32compat
    pmix
    event event_core
    hwloc
    ws2_32 iphlpapi advapi32 dbghelp shlwapi)
set_target_properties(prrte PROPERTIES
    C_STANDARD 11
    C_EXTENSIONS OFF
    OUTPUT_NAME prrte
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")

# ---- PRRTE tools --------------------------------------------------
# prte.exe: main() -> prte(); also copied to prterun.exe/mpirun.exe at
# package time (argv[0] selects the personality)
add_executable(prte_tool "${PRTE_SOURCE_DIR}/src/tools/prte/main.c")
target_link_libraries(prte_tool PRIVATE prrte)
set_target_properties(prte_tool PROPERTIES
    OUTPUT_NAME prte
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

add_executable(prted_tool "${PRTE_SOURCE_DIR}/src/tools/prted/prted.c")
target_link_libraries(prted_tool PRIVATE prrte)
set_target_properties(prted_tool PROPERTIES
    OUTPUT_NAME prted
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

add_executable(prte_submit
    "${PRTE_SOURCE_DIR}/src/tools/prun/main.c"
    "${PRTE_SOURCE_DIR}/src/tools/prun/prun.c")
target_link_libraries(prte_submit PRIVATE prrte)
target_include_directories(prte_submit PRIVATE
    "${PRTE_SOURCE_DIR}/src/tools/prun")
set_target_properties(prte_submit PROPERTIES
    OUTPUT_NAME prte-submit
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

add_executable(pterm "${PRTE_SOURCE_DIR}/src/tools/pterm/pterm.c")
target_link_libraries(pterm PRIVATE prrte)
set_target_properties(pterm PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

add_executable(prte_info "${PRTE_SOURCE_DIR}/src/tools/prte_info/prte_info.c")
target_link_libraries(prte_info PRIVATE prrte)
set_target_properties(prte_info PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

foreach(_t prte_tool prted_tool prte_submit pterm prte_info)
    # mirror the prrte include set, minus PRTE_BUILDING (consumers import)
    target_include_directories(${_t} PRIVATE
        "${PRTE_GEN_DIR}"
        "${PRTE_GEN_DIR}/include"
        "${PRTE_GEN_DIR}/src/include"
        "${PRTE_SOURCE_DIR}"
        "${PRTE_SOURCE_DIR}/src"
        "${PRTE_SOURCE_DIR}/src/include"
        "${PMIX_GEN_DIR}"
        "${PMIX_GEN_DIR}/src/include"
        "${PMIX_SOURCE_DIR}"
        "${PMIX_SOURCE_DIR}/src"
        "${PMIX_SOURCE_DIR}/include"
        "${CMAKE_BINARY_DIR}/pmix/include"
        "${CMAKE_BINARY_DIR}/pmix"
        "${CMAKE_SOURCE_DIR}/opal/win32/include"
        "${OPAL_WIN32_FWD_DIR}"
        "${CMAKE_BINARY_DIR}/deps-build/libevent/include"
        "${_le_src}/include"
        "${CMAKE_BINARY_DIR}/deps-build/hwloc/include"
        "${_hwloc_src}/include")
    target_compile_options(${_t} PRIVATE /experimental:c11atomics)
    target_link_libraries(${_t} PRIVATE
        pmix_win32compat
        pmix
        event event_core
        hwloc
        ws2_32 iphlpapi advapi32 dbghelp shlwapi)
    set_target_properties(${_t} PROPERTIES
        C_STANDARD 11
        C_EXTENSIONS OFF)
endforeach()

# argv[0]-selected personalities: prterun is the prte binary; prun is
# prte-submit; pterm is prte-term per upstream naming.  mpirun/mpiexec
# are NOT the prte binary - they are the ompi wrapper (below) which
# sets PRTE_MCA_personality=ompi and execs prterun.
add_custom_command(TARGET prte_tool POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
            "$<TARGET_FILE:prte_tool>"
            "${CMAKE_BINARY_DIR}/bin/$<CONFIG>/prterun.exe")

# ---- ompi mpirun/mpiexec wrapper: sets the ompi personality envars and
# execs prterun (see ompi/tools/mpirun/main.c) -------------------------
string(REPLACE ";" " " _opal_fws "${OPAL_MCA_FRAMEWORKS}")
string(REPLACE ";" " " _ompi_fws "${OMPI_MCA_FRAMEWORKS}")
add_executable(mpirun "${OMPI_SOURCE_DIR}/ompi/tools/mpirun/main.c")
target_compile_definitions(mpirun PRIVATE
    MCA_oshmem_FRAMEWORKS=""
    MCA_ompi_FRAMEWORKS="${_ompi_fws}"
    MCA_opal_FRAMEWORKS="${_opal_fws}")
target_include_directories(mpirun PRIVATE
    "${CMAKE_BINARY_DIR}/ompi/include"
    "${CMAKE_SOURCE_DIR}/ompi/include"
    "${CMAKE_BINARY_DIR}"
    "${CMAKE_SOURCE_DIR}"
    "${CMAKE_SOURCE_DIR}/opal/win32/include"
    "${OPAL_WIN32_FWD_DIR}"
    "${CMAKE_BINARY_DIR}/deps-build/hwloc/include"
    "${_hwloc_src}/include"
    "${CMAKE_BINARY_DIR}/deps-build/libevent/include"
    "${_le_src}/include")
target_link_libraries(mpirun PRIVATE open-pal)
set_target_properties(mpirun PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
add_custom_command(TARGET mpirun POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
            "$<TARGET_FILE:mpirun>"
            "${CMAKE_BINARY_DIR}/bin/$<CONFIG>/mpiexec.exe")
add_custom_command(TARGET prte_submit POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
            "$<TARGET_FILE:prte_submit>"
            "${CMAKE_BINARY_DIR}/bin/$<CONFIG>/prun.exe")
add_custom_command(TARGET pterm POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
            "$<TARGET_FILE:pterm>"
            "${CMAKE_BINARY_DIR}/bin/$<CONFIG>/prte-term.exe")
add_custom_command(TARGET prte_info POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
            "$<TARGET_FILE:prte_info>"
            "${CMAKE_BINARY_DIR}/bin/$<CONFIG>/prte-info.exe")

# ------------------------------------------------------------------
# subdirectories
# ------------------------------------------------------------------
add_subdirectory(opal)

# ------------------------------------------------------------------
# generated OMPI headers (build/w32/ompi/include/)
# ------------------------------------------------------------------
set(OMPI_GEN_INCLUDE_DIR "${CMAKE_BINARY_DIR}/ompi/include")
file(MAKE_DIRECTORY "${OMPI_GEN_INCLUDE_DIR}" "${OMPI_GEN_INCLUDE_DIR}/ompi")

# mpi.h -- substitute the autoheader #undef block for the MSVC/x86_64
# answers (cmake/gen_mpi_h.py); also emits the internal defines fragment
# used below for ompi_config.h.
execute_process(
    COMMAND ${Python3_EXECUTABLE}
            "${OMPI_SOURCE_DIR}/cmake/gen_mpi_h.py"
            "${OMPI_SOURCE_DIR}/ompi/include/mpi.h.in"
            "${OMPI_GEN_INCLUDE_DIR}/mpi.h"
            "${OMPI_MAJOR_VERSION}" "${OMPI_MINOR_VERSION}"
            "${OMPI_RELEASE_VERSION}"
            "${OMPI_MPI_STANDARD_VERSION}" "${OMPI_MPI_STANDARD_SUBVERSION}"
            --defs-out "${OMPI_GEN_INCLUDE_DIR}/ompi_config_block.h"
    RESULT_VARIABLE _mpih_rc
    ERROR_VARIABLE _mpih_err)
if(NOT _mpih_rc EQUAL 0)
    message(FATAL_ERROR "mpi.h generation failed: ${_mpih_err}")
endif()

# ompi_config.h -- copy the checked-in template and inject the config block
# (internal TUs include this first; mpi.h then skips its own copy).
file(READ "${OMPI_SOURCE_DIR}/ompi/include/ompi_config.h" _ompi_cfg)
file(READ "${OMPI_GEN_INCLUDE_DIR}/ompi_config_block.h" _ompi_block)
string(REPLACE "/* @OMPI_W32_CONFIG_BLOCK@ */" "${_ompi_block}"
       _ompi_cfg "${_ompi_cfg}")
file(WRITE "${OMPI_GEN_INCLUDE_DIR}/ompi_config.h" "${_ompi_cfg}")

# mpi_portable_platform.h -- a copy of opal's platform header
configure_file(
    "${OMPI_SOURCE_DIR}/opal/include/opal/opal_portable_platform_real.h"
    "${OMPI_GEN_INCLUDE_DIR}/mpi_portable_platform.h" COPYONLY)

# mpi-ext.h -- no mpiext components are built; emit the empty-extension file
# (mirrors config/ompi_ext.m4 EXT_CONFIGURE output)
file(WRITE "${OMPI_GEN_INCLUDE_DIR}/mpi-ext.h"
"/*
 * Generated by CMake -- no MPI extensions enabled on this build.
 */
#ifndef OMPI_MPI_EXT_H
#define OMPI_MPI_EXT_H 1

#if defined(c_plusplus) || defined(__cplusplus)
extern \"C\" {
#endif

#define OMPI_HAVE_MPI_EXT 0

#if defined(c_plusplus) || defined(__cplusplus)
}
#endif

#endif /* OMPI_MPI_EXT_H */
")

# ompi/version.h from version.h.in
set(OMPI_REPO_REV "${OPAL_SVN_REPO_REV}")
configure_file("${OMPI_SOURCE_DIR}/ompi/include/ompi/version.h.in"
               "${OMPI_GEN_INCLUDE_DIR}/ompi/version.h" @ONLY)

# mpif-c-constants.h -- ompi_mpi_init.c includes it unconditionally; with
# no Fortran bindings there is nothing to declare.
file(WRITE "${OMPI_GEN_INCLUDE_DIR}/mpif-c-constants.h"
"/* generated by CMake: no Fortran bindings in this build */
")

add_subdirectory(ompi)
#add_subdirectory(ompi/tools)

# ------------------------------------------------------------------
# mpicc wrapper-data file (build-tree paths; the installed layout gets
# an install-tree variant).  opal_wrapper resolves <appname>-wrapper-data.txt
# under opaldatadir (OPAL_PKGDATADIR env overrides for build-tree use).
# ------------------------------------------------------------------
file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/share/openmpi")
file(WRITE "${CMAKE_BINARY_DIR}/share/openmpi/mpicc-wrapper-data.txt"
"# mpicc wrapper data for the native Windows (MSVC) Open MPI build.
# Generated by CMake -- paths are absolute into the build/source trees.
project=Open MPI
project_short=OMPI
version=${OMPI_MAJOR_VERSION}.${OMPI_MINOR_VERSION}.${OMPI_RELEASE_VERSION}
language=C
compiler_env=CC
compiler_flags_env=CFLAGS
compiler=cl
compiler_flags_prefix=/nologo
# NOTE: no quotes around /I and /LIBPATH values -- the wrapper spawns the
# compiler via CreateProcess with a reconstructed command line, and embedded
# quotes survive into argv which breaks MSVC's /I parsing.  Build-tree paths
# must therefore not contain spaces.
preprocessor_flags=/DOMPI_IMPORTS /I${OMPI_GEN_INCLUDE_DIR} /I${OMPI_SOURCE_DIR}/ompi/include /I${CMAKE_BINARY_DIR}/opal/include /I${OMPI_SOURCE_DIR}/opal/include /I${OMPI_SOURCE_DIR}/opal/win32/include
compiler_flags=
linker_flags=/link /LIBPATH:${CMAKE_BINARY_DIR}/lib/Release
libs=open-mpi.lib open-pal.lib ws2_32.lib iphlpapi.lib advapi32.lib dbghelp.lib shlwapi.lib
libs_static=open-mpi.lib open-pal.lib pmix.lib ws2_32.lib iphlpapi.lib advapi32.lib dbghelp.lib shlwapi.lib
dyn_lib_file=open-mpi.dll
static_lib_file=
required_file=
includedir=${OMPI_GEN_INCLUDE_DIR}
libdir=${CMAKE_BINARY_DIR}/lib
")

# stage the show_help message catalogs so error paths actually print
file(GLOB_RECURSE _help_files
     "${OMPI_SOURCE_DIR}/opal/*/help-*.txt"
     "${OMPI_SOURCE_DIR}/ompi/*/help-*.txt")
foreach(_hf IN LISTS _help_files)
    file(COPY "${_hf}" DESTINATION "${CMAKE_BINARY_DIR}/share/openmpi")
endforeach()

enable_testing()
