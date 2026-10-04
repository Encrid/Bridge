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

# ---- phase 2: the backend seam and the wasm backend. docs/plan/PHASE2.md ----

find_program (WASM_TOOLS wasm-tools REQUIRED)

# wasmtime's C API, vendored: third_party/wasmtime/README.md
set (WASMTIME "${CMAKE_CURRENT_SOURCE_DIR}/third_party/wasmtime")
add_library (wasmtime SHARED IMPORTED)
target_include_directories (wasmtime INTERFACE "${WASMTIME}/include")
if (WIN32)
    set_target_properties (wasmtime PROPERTIES
        IMPORTED_LOCATION "${WASMTIME}/windows/wasmtime.dll"
        IMPORTED_IMPLIB "${WASMTIME}/windows/wasmtime.dll.lib")
else ()
    set_target_properties (wasmtime PROPERTIES
        IMPORTED_LOCATION "${WASMTIME}/linux/libwasmtime.so")
endif ()

# what the operating system is asked for, shared by the library, a backend and the tests
add_library (bridge-os STATIC src/os.c)
set_target_properties (bridge-os PROPERTIES
    C_STANDARD 11 C_STANDARD_REQUIRED YES POSITION_INDEPENDENT_CODE ON)
target_include_directories (bridge-os PUBLIC src)
if (UNIX)
    target_link_libraries (bridge-os PUBLIC dl pthread)
endif ()

# the wasm backend: a shared library exporting the sixteen entries
add_library (bridge-wasm SHARED backends/wasm/wasm.c)
set_target_properties (bridge-wasm PROPERTIES C_STANDARD 11 C_STANDARD_REQUIRED YES PREFIX "")
target_include_directories (bridge-wasm PRIVATE include)
target_link_libraries (bridge-wasm PRIVATE wasmtime bridge-os)
if (WIN32)
    # the backend loads wasmtime.dll from beside itself
    add_custom_command (TARGET bridge-wasm POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different
            "${WASMTIME}/windows/wasmtime.dll" "$<TARGET_FILE_DIR:bridge-wasm>")
endif ()

# test modules are wasm text, assembled here
set (MODULES "${CMAKE_CURRENT_BINARY_DIR}/modules")
file (GLOB WAT CONFIGURE_DEPENDS "tests/modules/*.wat")
set (WASM)
foreach (wat ${WAT})
    get_filename_component (name "${wat}" NAME_WE)
    add_custom_command (
        OUTPUT "${MODULES}/${name}.wasm"
        COMMAND "${CMAKE_COMMAND}" -E make_directory "${MODULES}"
        COMMAND "${WASM_TOOLS}" parse "${wat}" -o "${MODULES}/${name}.wasm"
        DEPENDS "${wat}"
        COMMENT "assembling ${name}.wat"
        VERBATIM)
    list (APPEND WASM "${MODULES}/${name}.wasm")
endforeach ()
add_custom_target (modules ALL DEPENDS ${WASM})

function (backend_test name)
    add_executable (${name} "tests/${name}.c" src/backend.c ${ARGN})
    set_target_properties (${name} PROPERTIES C_STANDARD 11 C_STANDARD_REQUIRED YES)
    target_include_directories (${name} PRIVATE include)
    target_link_libraries (${name} PRIVATE bridge-os)
    target_compile_definitions (${name} PRIVATE
        "BACKEND_PATH=\"$<TARGET_FILE:bridge-wasm>\"" "MODULE_DIR=\"${MODULES}\"")
    add_dependencies (${name} bridge-wasm modules)
    add_test (NAME ${name} COMMAND ${name})
endfunction ()

# one block backing a memory in two stores at once: the question the phase opens with
backend_test (adopt)
