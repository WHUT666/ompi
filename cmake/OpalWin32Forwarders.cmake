# Copyright (c) 2026      Amazon.com, Inc. or its affiliates.
#                         All Rights reserved.
# $COPYRIGHT$
#
# Additional copyrights may follow
#
# $HEADER$
#
# Generates the "forwarder" headers used by the native Windows build.
#
# Several POSIX headers that the Open MPI sources include by name
# (<time.h>, <signal.h>, <sys/types.h>, ...) already exist in the MSVC
# toolchain with slightly different content.  MSVC has no
# #include_next, so the shadow headers forward to the real toolchain
# header by absolute path and then add the missing POSIX bits.
#
# The forwarder bodies live in the source tree under
#   opal/win32/incfwd/<name>.h.in
# and contain @OPAL_CRT_UCRT@ / @OPAL_CRT_MSVC@ / @OPAL_CRT_UM@ /
# @OPAL_CRT_SHARED@ placeholders that are substituted with the
# toolchain include directories detected at configure time.
#
#   opal_win32_generate_forwarders(<output dir>
#       <MSVC inc dir> <UCRT inc dir> <UM inc dir> <SHARED inc dir>)

function(opal_win32_generate_forwarders outdir msvc_inc ucrt_inc um_inc shared_inc)
    set(OPAL_CRT_MSVC   "${msvc_inc}")
    set(OPAL_CRT_UCRT   "${ucrt_inc}")
    set(OPAL_CRT_UM     "${um_inc}")
    set(OPAL_CRT_SHARED "${shared_inc}")

    set(srcdir "${OMPI_SOURCE_DIR}/opal/win32/incfwd")
    file(GLOB_RECURSE templates RELATIVE "${srcdir}" "${srcdir}/*.in")
    if(NOT templates)
        message(FATAL_ERROR "no forwarder templates under ${srcdir}")
    endif()
    foreach(t IN LISTS templates)
        string(REGEX REPLACE "\\.in$" "" rel "${t}")
        get_filename_component(_dir "${outdir}/${rel}" DIRECTORY)
        file(MAKE_DIRECTORY "${_dir}")
        configure_file("${srcdir}/${t}" "${outdir}/${rel}" @ONLY)
    endforeach()
    list(LENGTH templates _n)
    message(STATUS "generated ${_n} POSIX forwarder headers in ${outdir}")
endfunction()
