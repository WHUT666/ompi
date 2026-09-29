# Copyright (c) 2026      Amazon.com, Inc. or its affiliates.
#                         All Rights reserved.
# $COPYRIGHT$
#
# Additional copyrights may follow
#
# $HEADER$
# SPDX-License-Identifier: BSD-3-Clause-Open-MPI
#
# libopen-pal.dll -- the OPAL portability layer, Windows build.

# ------------------------------------------------------------------
# generated sources
# ------------------------------------------------------------------
# show_help_content.c embeds every help-*.txt file in the tree; the
# generator mirrors opal/util/Makefile.am's rule but writes into the
# build directory so the source tree stays clean.
set(SHOW_HELP_CONTENT "${CMAKE_BINARY_DIR}/generated/show_help_content.c")
file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/generated")
add_custom_command(
    OUTPUT "${SHOW_HELP_CONTENT}"
    COMMAND "${Python3_EXECUTABLE}"
        "${CMAKE_CURRENT_SOURCE_DIR}/util/convert-help-files-to-c-code.py"
        --roots "${CMAKE_SOURCE_DIR}/opal" "${CMAKE_SOURCE_DIR}/ompi"
        --out "${SHOW_HELP_CONTENT}"
    DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/util/convert-help-files-to-c-code.py"
    COMMENT "Generating show_help_content.c"
    VERBATIM)

# keyval_lex.l -> keyval_lex.c (win_flex; AM_LFLAGS = -Popal_util_keyval_yy)
find_program(WIN_FLEX_EXECUTABLE NAMES win_flex flex)
if(NOT WIN_FLEX_EXECUTABLE)
    message(FATAL_ERROR
        "flex not found -- install win_flex (winget install WinFlexBison.win_flex_bison)")
endif()
set(KEYVAL_LEX_C "${CMAKE_BINARY_DIR}/generated/keyval_lex.c")
add_custom_command(
    OUTPUT "${KEYVAL_LEX_C}"
    COMMAND "${WIN_FLEX_EXECUTABLE}" -Popal_util_keyval_yy
            -o "${KEYVAL_LEX_C}"
            "${CMAKE_CURRENT_SOURCE_DIR}/util/keyval/keyval_lex.l"
    DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/util/keyval/keyval_lex.l"
    COMMENT "Generating keyval_lex.c (flex)"
    VERBATIM)

# ------------------------------------------------------------------
# core sources
# ------------------------------------------------------------------
# (Explicit lists for now; cmake/harvest_sources.py can harvest these
# from Makefile.am later once the build is stable.)
set(OPAL_SOURCES
    class/opal_bitmap.c
    class/opal_cstring.c
    class/opal_free_list.c
    class/opal_hash_table.c
    class/opal_hotel.c
    class/opal_list.c
    class/opal_object.c
    class/opal_graph.c
    class/opal_lifo.c
    class/opal_fifo.c
    class/opal_pointer_array.c
    class/opal_value_array.c
    class/opal_ring_buffer.c
    class/opal_rb_tree.c
    class/opal_interval_tree.c

    datatype/opal_convertor.c
    datatype/opal_convertor_raw.c
    datatype/opal_copy_functions.c
    datatype/opal_copy_functions_heterogeneous.c
    datatype/opal_datatype_add.c
    datatype/opal_datatype_clone.c
    datatype/opal_datatype_copy.c
    datatype/opal_datatype_create.c
    datatype/opal_datatype_create_contiguous.c
    datatype/opal_datatype_destroy.c
    datatype/opal_datatype_dump.c
    datatype/opal_datatype_fake_stack.c
    datatype/opal_datatype_get_count.c
    datatype/opal_datatype_module.c
    datatype/opal_datatype_monotonic.c
    datatype/opal_datatype_optimize.c
    datatype/opal_datatype_pack.c
    datatype/opal_datatype_pack_accelerator.c
    datatype/opal_datatype_position.c
    datatype/opal_datatype_resize.c
    datatype/opal_datatype_unpack.c
    datatype/opal_datatype_unpack_accelerator.c

    memoryhooks/memory.c

    runtime/opal_finalize.c
    runtime/opal_finalize_core.c
    runtime/opal_info_support.c
    runtime/opal_init.c
    runtime/opal_init_core.c
    runtime/opal_params.c
    runtime/opal_params_core.c
    runtime/opal_progress.c
    runtime/opal_progress_threads.c

    util/ethtool.c
    util/event.c
    util/if.c
    util/net.c
    util/info_subscriber.c
    util/info.c
    util/alfg.c
    util/arch.c
    util/argv.c
    util/basename.c
    util/bipartite_graph.c
    util/cmd_line.c
    util/error.c
    util/fd.c
    util/few.c
    util/keyval_parse.c
    util/malloc.c
    util/numtostr.c
    util/opal_environ.c
    util/opal_getcwd.c
    util/os_dirpath.c
    util/os_path.c
    util/output.c
    util/path.c
    util/printf.c
    util/proc.c
    util/sha256.c
    util/show_help.c
    util/stacktrace.c
    util/string_copy.c
    util/sys_limits.c
    util/uri.c

    util/json/3rd-party/json.c
    util/json/opal_json.c

    "${KEYVAL_LEX_C}"

    mca/base/mca_base_alias.c
    mca/base/mca_base_close.c
    mca/base/mca_base_cmd_line.c
    mca/base/mca_base_component_compare.c
    mca/base/mca_base_component_find.c
    mca/base/mca_base_component_repository.c
    mca/base/mca_base_components_close.c
    mca/base/mca_base_components_open.c
    mca/base/mca_base_components_register.c
    mca/base/mca_base_components_select.c
    mca/base/mca_base_event.c
    mca/base/mca_base_event_dispatch.c
    mca/base/mca_base_event_memory.c
    mca/base/mca_base_framework.c
    mca/base/mca_base_list.c
    mca/base/mca_base_open.c
    mca/base/mca_base_parse_paramfile.c
    mca/base/mca_base_pvar.c
    mca/base/mca_base_var.c
    mca/base/mca_base_var_enum.c
    mca/base/mca_base_var_group.c

    # Windows compatibility layer
    win32/opal_win32_compat.c
    win32/opal_win32_fs.c
    win32/opal_win32_misc.c
    win32/opal_win32_proc.c
    win32/opal_win32_pthread.c
    win32/opal_win32_sem.c
    win32/opal_win32_socket.c

    "${SHOW_HELP_CONTENT}"
)

# ------------------------------------------------------------------
# framework base dirs and selected static components
# ------------------------------------------------------------------
# populates OPAL_MCA_SOURCES (see opal/mca/CMakeLists.txt)
add_subdirectory(mca)

add_library(open-pal SHARED ${OPAL_SOURCES} ${OPAL_MCA_SOURCES})
target_link_options(open-pal PRIVATE /DEBUG)
add_library(ompi::open-pal ALIAS open-pal)

# OPAL_DECLSPEC=dllexport applies only when building libopen-pal itself
target_compile_definitions(open-pal PRIVATE OPAL_BUILDING=1)

# repo root is the include root: sources include "opal/mca/..." etc.
# The deps' own generated config headers (event2/event-config.h,
# hwloc/autogen/config.h) land in deps-build/*/include and must match
# the compiled libraries -- do not substitute the nmake copies.
target_include_directories(open-pal PRIVATE
    "${CMAKE_SOURCE_DIR}"
    "${CMAKE_BINARY_DIR}"
    "${CMAKE_BINARY_DIR}/deps-build/hwloc/include"
    "${_hwloc_src}/include"
    "${CMAKE_BINARY_DIR}/deps-build/libevent/include"
    "${_le_src}/include")

target_link_libraries(open-pal PRIVATE
    ws2_32 iphlpapi advapi32 dbghelp shlwapi
    event_shared hwloc pmix)

set_target_properties(open-pal PROPERTIES
    OUTPUT_NAME open-pal
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")

install(TARGETS open-pal
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
    LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}")

# ---- opal_wrapper: the mpicc/opalcc driver executable ----
# argv[0] basename selects the *-wrapper-data.txt config in opaldatadir;
# install/copy it as mpicc.exe, opalcc.exe, ...
add_executable(opal_wrapper tools/wrappers/opal_wrapper.c)
# mirrors the autotools DEFS="-DEXEEXT=\"$(EXEEXT)\"" on this target
target_compile_definitions(opal_wrapper PRIVATE EXEEXT=".exe")
target_include_directories(opal_wrapper PRIVATE
    "${CMAKE_SOURCE_DIR}"
    "${CMAKE_BINARY_DIR}"
    "${CMAKE_BINARY_DIR}/deps-build/hwloc/include"
    "${_hwloc_src}/include"
    "${CMAKE_BINARY_DIR}/deps-build/libevent/include"
    "${_le_src}/include")
target_link_libraries(opal_wrapper PRIVATE
    open-pal ws2_32 iphlpapi advapi32 dbghelp shlwapi)
set_target_properties(opal_wrapper PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

install(TARGETS opal_wrapper
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")
