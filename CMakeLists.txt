# Bridge-ASM700.
#
#   cmake --preset windows && cmake --build --preset windows && ctest --preset windows
#
# Phase 1 builds no Bridge-ASM700 yet: a patched flecs, and the two tests that prove the patch.
# docs/plan/PHASE1.md.

cmake_minimum_required (VERSION 3.21)

project (bridge-asm700
    DESCRIPTION "An orchestrator that runs modules over shared ECS storage"
    LANGUAGES C)

enable_testing ()

find_package (Python3 REQUIRED COMPONENTS Interpreter)

set (FLECS_SRC "${CMAKE_CURRENT_SOURCE_DIR}/third_party/flecs")
set (PATCH "${CMAKE_CURRENT_SOURCE_DIR}/tools/patch-flecs.py")

# flecs, patched at build time from the pristine copy. The files in third_party/ are never
# edited; the script is the record of what changes. Arguments after the target name are groups
# to leave unpatched, which only the tests that must fail use.
function (patched_flecs target)
    set (out "${CMAKE_CURRENT_BINARY_DIR}/${target}")
    set (omit)
    foreach (group ${ARGN})
        list (APPEND omit --omit ${group})
    endforeach ()
    add_custom_command (
        OUTPUT "${out}/flecs.c" "${out}/flecs.h" "${out}/flecs_confine.h"
        COMMAND "${Python3_EXECUTABLE}" "${PATCH}" "${FLECS_SRC}" "${out}" ${omit}
        DEPENDS "${PATCH}" "${FLECS_SRC}/flecs.c" "${FLECS_SRC}/flecs.h"
        COMMENT "patching flecs into ${target}"
        VERBATIM)
    # built on its own terms: its own warnings, none of ours
    add_library (${target} STATIC "${out}/flecs.c")
    set_target_properties (${target} PROPERTIES C_STANDARD 99 C_STANDARD_REQUIRED YES)
    target_include_directories (${target} PUBLIC "${out}")
    if (WIN32)
        target_link_libraries (${target} PUBLIC ws2_32 dbghelp)
    else ()
        target_link_libraries (${target} PUBLIC m pthread)
    endif ()
endfunction ()

patched_flecs (flecs)
patched_flecs (flecs-unconfined allocator)
patched_flecs (flecs-nohook hook)

function (flecs_test name source lib)
    add_executable (${name} "tests/${source}")
    set_target_properties (${name} PROPERTIES C_STANDARD 99 C_STANDARD_REQUIRED YES)
    target_link_libraries (${name} PRIVATE ${lib})
    add_test (NAME ${name} COMMAND ${name})
endfunction ()

flecs_test (confined confined.c flecs)
flecs_test (rowmove rowmove.c flecs)

# The same two tests against a flecs missing half the patch. Each has to fail, and by saying
# what it found rather than by crashing: a test that cannot fail proves nothing.
flecs_test (confined-unconfined confined.c flecs-unconfined)
flecs_test (rowmove-nohook rowmove.c flecs-nohook)
set_tests_properties (confined-unconfined PROPERTIES
    PASS_REGULAR_EXPRESSION "FAIL: [0-9]+ pointers escaped the block")
set_tests_properties (rowmove-nohook PROPERTIES
    PASS_REGULAR_EXPRESSION "FAIL: [0-9]+ row mismatches")

# The script declines a flecs it was not written for.
set (other "${CMAKE_CURRENT_BINARY_DIR}/other-version")
file (READ "${FLECS_SRC}/flecs.h" header)
string (REPLACE "#define FLECS_VERSION_PATCH 6" "#define FLECS_VERSION_PATCH 7" header "${header}")
file (WRITE "${other}/flecs.h" "${header}")
configure_file ("${FLECS_SRC}/flecs.c" "${other}/flecs.c" COPYONLY)
add_test (NAME patch-version
    COMMAND "${Python3_EXECUTABLE}" "${PATCH}" "${other}" "${other}/out")
set_tests_properties (patch-version PROPERTIES
    PASS_REGULAR_EXPRESSION "version 4.1.7, and this script was written for 4.1.6")
