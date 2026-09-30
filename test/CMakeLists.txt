# ------------------------------------------------------------------
# Windows-port test programs.
#
# Builds a curated set of test/simple/*.c against open-mpi and
# registers multi-process runs with CTest.  The runtime needs the
# same environment as the interactive smoke tests:
#   PATH           -> bin/Release + lib/Release
#   OPAL_PKGDATADIR-> share/openmpi (help files)
#   OMPI_PRTERUN   -> prterun.exe
#
# NOTE: mpirun currently deadlocks when its own stdout is a pipe
# (pre-existing launcher limitation in the Windows port), so each
# test runs through a generated cmake -P wrapper that redirects
# mpirun output to a log file, echoes it back, and returns mpirun's
# real exit code to CTest.
# ------------------------------------------------------------------

option(OMPI_BUILD_TESTS "Build MPI test programs wired into CTest" ON)

if(NOT OMPI_BUILD_TESTS)
    return()
endif()

set(_win_mpi_tests
    win_rma
    mpi_io
    mpi_io_nb
)

foreach(t IN LISTS _win_mpi_tests)
    add_executable(${t} "simple/${t}.c")
    target_include_directories(${t} PRIVATE
        "${CMAKE_BINARY_DIR}/ompi/include")
    # mpi.h annotates predefined handle objects (ompi_mpi_comm_world,
    # ompi_mpi_int, ...) with OMPI_DECLSPEC; consumers need the
    # dllimport spelling to resolve them from open-mpi.dll
    target_compile_definitions(${t} PRIVATE OMPI_IMPORTS)
    target_link_libraries(${t} PRIVATE open-mpi)
    set_target_properties(${t} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/test")
endforeach()

# ---- ctest registration ------------------------------------------------
file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/test")

foreach(t IN LISTS _win_mpi_tests)
    set(NP 2)
    set(TEST_NAME "${t}_np2")
    configure_file("${CMAKE_CURRENT_SOURCE_DIR}/run_mpi_test.cmake.in"
        "${CMAKE_BINARY_DIR}/test/${t}_np2.cmake" @ONLY)
    add_test(NAME ${t}_np2
        COMMAND "${CMAKE_COMMAND}"
            -DMPIRUN=$<TARGET_FILE:mpirun>
            -DPRTERUN=${CMAKE_BINARY_DIR}/bin/$<CONFIG>/prterun.exe
            -DBIN_DIR=${CMAKE_BINARY_DIR}/bin/$<CONFIG>
            -DLIB_DIR=${CMAKE_BINARY_DIR}/lib/$<CONFIG>
            -DTEST_EXE=$<TARGET_FILE:${t}>
            -P "${CMAKE_BINARY_DIR}/test/${t}_np2.cmake")
    set_tests_properties(${t}_np2 PROPERTIES
        FAIL_REGULAR_EXPRESSION "FAIL|RESULT FAIL")
endforeach()
