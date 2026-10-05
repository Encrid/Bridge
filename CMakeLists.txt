# Bridge-ASM700.
#
#   cmake --preset windows && cmake --build --preset windows && ctest --preset windows
#
# Each phase adds its part below: docs/plan/.
#

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

# the wasm backend: a shared library exporting the seventeen entries
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

# Bridge-ASM700 itself, as far as it exists
add_library (bridge STATIC src/bridge.c src/lifecycle.c src/config.c src/store.c src/abi.c
    src/schedule.c src/pool.c src/backend.c)
set_target_properties (bridge PROPERTIES C_STANDARD 11 C_STANDARD_REQUIRED YES)
target_include_directories (bridge PUBLIC include PRIVATE src)
target_link_libraries (bridge PRIVATE bridge-os flecs)

# a host that links the library and nothing else, run once per claim of the phase
add_executable (phase2 tests/phase2.c)
set_target_properties (phase2 PROPERTIES C_STANDARD 11 C_STANDARD_REQUIRED YES)
target_link_libraries (phase2 PRIVATE bridge)
target_compile_definitions (phase2 PRIVATE
    "BACKEND_PATH=\"$<TARGET_FILE:bridge-wasm>\"" "MODULE_DIR=\"${MODULES}\"")
add_dependencies (phase2 bridge-wasm modules)

# every case is checked by what was said, and by the process still being there to say "alive"
function (phase2_case name expect)
    add_test (NAME phase2-${name} COMMAND phase2 ${name})
    set_tests_properties (phase2-${name} PROPERTIES PASS_REGULAR_EXPRESSION "${expect}.*alive")
endfunction ()

phase2_case (ok        "test:ok@0.1.0: loaded")
phase2_case (nolimit   "test:ok@0.1.0: loaded")
phase2_case (image     "test:image@0.1.0: loaded")
phase2_case (oob       "module 'test:oob@0.1.0' did not load: its init trapped: out of bounds memory access")
phase2_case (spin      "module 'test:spin@0.1.0' did not load: its init trapped: interrupt")
phase2_case (grow      "module 'test:grow@0.1.0' did not load: its init answered 7")
phase2_case (big       "module 'test:big@0.1.0' did not load: cannot instantiate")
phase2_case (shape     "module 'test:shape@0.1.0' did not load: its export 'init' is not .. -> i32")
phase2_case (noinit    "module 'test:noinit@0.1.0' did not load: no exported function 'init'")
phase2_case (abi       "module 'test:ok@0.1.0' is refused: it wants ABI 2 and this is ABI 1")
phase2_case (nobackend "module 'test:ok@0.1.0' is refused: no backend named 'native'")
phase2_case (unset     "module 'test:ok@0.1.0' is refused: memory_limit is unset")
phase2_case (stop      "test:ok@0.1.0: loaded.*the bridge has stopped: module 'test:oob@0.1.0' failed and its on_error is fail.*test:ok@0.1.0: not loaded")
phase2_case (nolib     "backend 'wasm': cannot open backend library 'no-such-backend-library'.*bridge_create refused")
phase2_case (nosink    "refused: no sink")

# ---- phase 3: storage, and modules compiled from C. docs/plan/PHASE3.md ----

find_program (ZIG zig REQUIRED)

# Binaryen merges the accessors into a compiled module and inlines them: wasm.md, *A compiled
# module reaches them through accessors*. Found on the machine, or fetched at its pin.
set (BINARYEN "${CMAKE_CURRENT_BINARY_DIR}/binaryen-version_133/bin")
find_program (WASM_MERGE wasm-merge HINTS "$ENV{BINARYEN_ROOT}/bin" "${BINARYEN}")
if (NOT WASM_MERGE AND WIN32)
    file (DOWNLOAD
        "https://github.com/WebAssembly/binaryen/releases/download/version_133/binaryen-version_133-x86_64-windows.tar.gz"
        "${CMAKE_CURRENT_BINARY_DIR}/binaryen.tar.gz"
        EXPECTED_HASH SHA256=17a2cbeac6b5693c5fbafab3838d3c65fd9c1eb38b05f5baec6c657e8c84995b)
    file (ARCHIVE_EXTRACT INPUT "${CMAKE_CURRENT_BINARY_DIR}/binaryen.tar.gz"
        DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")
endif ()
find_program (WASM_MERGE wasm-merge HINTS "$ENV{BINARYEN_ROOT}/bin" "${BINARYEN}" REQUIRED)
find_program (WASM_OPT wasm-opt HINTS "$ENV{BINARYEN_ROOT}/bin" "${BINARYEN}" REQUIRED)
set (FEATURES --enable-multimemory --enable-bulk-memory --enable-sign-ext
    --enable-nontrapping-float-to-int --enable-mutable-globals)

# The accessors, and the same without the row < count test: only the measurement uses that.
set (ACC_WAT "${CMAKE_CURRENT_SOURCE_DIR}/include/bridge-wasm.wat")
file (READ "${ACC_WAT}" accessors)
string (REPLACE "(if (i32.ge_u (local.get $row) (local.get $count)) (then (unreachable)))" ""
    unchecked "${accessors}")
if (unchecked STREQUAL accessors)
    message (FATAL_ERROR "bridge-wasm.wat: the row < count test is not where the build expects it")
endif ()
file (WRITE "${CMAKE_CURRENT_BINARY_DIR}/bridge-wasm-unchecked.wat" "${unchecked}")
set_property (DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${ACC_WAT}")
foreach (kind checked unchecked)
    if (kind STREQUAL checked)
        set (wat "${ACC_WAT}")
    else ()
        set (wat "${CMAKE_CURRENT_BINARY_DIR}/bridge-wasm-unchecked.wat")
    endif ()
    add_custom_command (
        OUTPUT "${MODULES}/acc-${kind}.wasm"
        COMMAND "${CMAKE_COMMAND}" -E make_directory "${MODULES}"
        COMMAND "${WASM_TOOLS}" parse "${wat}" -o "${MODULES}/acc-${kind}.wasm"
        DEPENDS "${wat}"
        VERBATIM)
endforeach ()

# A module in C: compiled, merged with the accessors, inlined, and stripped of their exports.
function (wasm_module name source kind)
    set (src "${CMAKE_CURRENT_SOURCE_DIR}/tests/modules/${source}.c")
    set (defines)
    foreach (define ${ARGN})
        list (APPEND defines "-D${define}")
    endforeach ()
    add_custom_command (
        OUTPUT "${MODULES}/${name}.wasm"
        COMMAND "${ZIG}" cc -target wasm32-freestanding -O2 -nostdlib ${defines}
            "-I${CMAKE_CURRENT_SOURCE_DIR}/include" "-Wl,--no-entry"
            -o "${MODULES}/${name}.raw.wasm" "${src}"
        COMMAND "${WASM_MERGE}" "${MODULES}/${name}.raw.wasm" main
            "${MODULES}/acc-${kind}.wasm" acc ${FEATURES} -o "${MODULES}/${name}.merged.wasm"
        COMMAND "${WASM_OPT}" "${MODULES}/${name}.merged.wasm" ${FEATURES} -O3
            "--remove-exports=acc_*" --remove-unused-module-elements
            -o "${MODULES}/${name}.wasm"
        DEPENDS "${src}" "${MODULES}/acc-${kind}.wasm"
            "${CMAKE_CURRENT_SOURCE_DIR}/tests/modules/game.h"
            "${CMAKE_CURRENT_SOURCE_DIR}/tests/modules/roster.h"
            "${CMAKE_CURRENT_SOURCE_DIR}/include/bridge-wasm.h"
            "${CMAKE_CURRENT_SOURCE_DIR}/include/bridge-abi.h"
        COMMENT "building module ${name}"
        VERBATIM)
    set_property (GLOBAL APPEND PROPERTY C_MODULES "${MODULES}/${name}.wasm")
endfunction ()

foreach (name mover reader refs fp wildcard spin3 flood chatty rows)
    wasm_module (${name} ${name} checked)
endforeach ()
wasm_module (rows-unchecked rows unchecked)
get_property (C_MODULES GLOBAL PROPERTY C_MODULES)
add_custom_target (c-modules ALL DEPENDS ${C_MODULES})

add_executable (phase3 tests/phase3.c)
set_target_properties (phase3 PROPERTIES C_STANDARD 11 C_STANDARD_REQUIRED YES)
target_link_libraries (phase3 PRIVATE bridge)
target_compile_definitions (phase3 PRIVATE
    "BACKEND_PATH=\"$<TARGET_FILE:bridge-wasm>\"" "MODULE_DIR=\"${MODULES}\"")
add_dependencies (phase3 bridge-wasm c-modules)

function (phase3_case name expect)
    add_test (NAME phase3-${name} COMMAND phase3 ${name} ${ARGN})
    set_tests_properties (phase3-${name} PROPERTIES PASS_REGULAR_EXPRESSION "${expect}.*alive")
endfunction ()

# no crossing per access: the built system loads and stores, and calls nothing
add_test (NAME phase3-nocall
    COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/tools/nocall.py"
        "${WASM_TOOLS}" "${MODULES}/mover.wasm" move)

phase3_case (share       "sum 250500 0.*moved 2 1000.*sum 251000 0.*sum 251500 0")
phase3_case (spans       "view:reader@0.1.0/sum: saw 1 500.*game:mover@0.1.0/count: moved 2 1000")
phase3_case (log         "record.0. view:reader@0.1.0/sum: sum 250500 0")
phase3_case (mirror      "mirror 100 0.*mirror 100 0.*mirror 100 0")
phase3_case (fingerprint "module 'game:fp@0.1.0' did not load: component 'game:Position' has another layout here: fingerprint [0-9a-f]+, and it was declared with [0-9a-f]+")
phase3_case (grant       "module 'view:reader@0.1.0' did not load: it may not declare 'game:Position': that is another namespace's, and no grant names it")
phase3_case (wildcard    "module 'game:wildcard@0.1.0' did not load: term 0 of system 'any' is a wildcard")
phase3_case (fuel        "test:spin@0.1.0/forever: system 'forever' failed: trapped: all fuel consumed.*still here.*all fuel consumed.*still here")
phase3_case (ceiling     "system 'flood' failed: spawn: its spawns of 3000 between two barriers are used up.*system 'flood' failed: spawn: .budget. entities of 4096 is reached")
phase3_case (ring        "line 15 0.*test:chatty@0.1.0/-: 84 log lines were dropped: its ring holds 16")
phase3_case (nospans     "module 'view:reader@0.1.0' is stopped: what its queries match no longer fits in its spans of 16 bytes.*moved 2 1000")
phase3_case (nocolumns   "the bridge has stopped: .budget. columns of 60000 bytes ran out.*game:mover@0.1.0: not loaded.*update refused" 60000)
phase3_case (nophase     "phase 'render' is not one the deployment declared")
phase3_case (nophases    "the deployment declares no phase.*bridge_create refused")
phase3_case (nobudget    ".budget. columns is unset.*bridge_create refused")
# a measurement, not a claim: it passes by running. docs/bridge/diagnostics/validation.md has what it said
phase3_case (bench       "rows-unchecked  no fuel   [0-9.]+ ns a row")

# ---- phase 4: the native backend. docs/plan/PHASE4.md ----

add_library (bridge-native SHARED backends/native/native.c)
set_target_properties (bridge-native PROPERTIES C_STANDARD 11 C_STANDARD_REQUIRED YES PREFIX "")
target_include_directories (bridge-native PRIVATE include)
target_link_libraries (bridge-native PRIVATE bridge-os)

# a native module is a shared library, built by the compiler that builds everything else
foreach (name nmover nflood)
    add_library (${name} MODULE tests/modules/native/${name}.c)
    set_target_properties (${name} PROPERTIES C_STANDARD 11 C_STANDARD_REQUIRED YES PREFIX "")
    target_include_directories (${name} PRIVATE include)
endforeach ()

add_executable (phase4 tests/phase4.c)
set_target_properties (phase4 PROPERTIES C_STANDARD 11 C_STANDARD_REQUIRED YES)
target_link_libraries (phase4 PRIVATE bridge)
target_compile_definitions (phase4 PRIVATE
    "BACKEND_PATH=\"$<TARGET_FILE:bridge-wasm>\"" "NATIVE_PATH=\"$<TARGET_FILE:bridge-native>\""
    "NMOVER_PATH=\"$<TARGET_FILE:nmover>\"" "NFLOOD_PATH=\"$<TARGET_FILE:nflood>\""
    "MODULE_DIR=\"${MODULES}\"")
add_dependencies (phase4 bridge-wasm bridge-native nmover nflood c-modules)

function (phase4_case name expect)
    add_test (NAME phase4-${name} COMMAND phase4 ${name})
    set_tests_properties (phase4-${name} PROPERTIES PASS_REGULAR_EXPRESSION "${expect}.*alive")
endfunction ()

phase4_case (raw       "game:mover@0.1.0/tally: native tally 2 1000.*native tally 2 1000")
phase4_case (mixed     "view:reader@0.1.0/sum: sum 250500 0.*native tally 2 1000.*sum 251000 0")
phase4_case (untrusted "module 'game:mover@0.1.0' is refused: trusted = false needs 'sandbox', which backend 'native' lacks")
phase4_case (limit     "module 'game:mover@0.1.0' is refused: fuel needs 'meter', which backend 'native' lacks")
phase4_case (nolib     "module 'game:mover@0.1.0' did not load: cannot open the library 'no-such-library'")
phase4_case (refuse    "test:flood@0.1.0/flood: system 'flood' failed: spawn: its spawns of 1024 between two barriers are used up")
# before its library is opened: the path names nothing, and nothing says it was looked for
set_tests_properties (phase4-untrusted phase4-limit PROPERTIES FAIL_REGULAR_EXPRESSION "cannot open")

# ---- phase 5: the schedule, observers, threads. docs/plan/PHASE5.md ----

foreach (name halver twostep watcher chain badobs events fixer nested nested2 namer spawner)
    wasm_module (${name} ${name} checked)
endforeach ()
get_property (C_MODULES GLOBAL PROPERTY C_MODULES)
add_custom_target (c-modules-5 ALL DEPENDS ${C_MODULES})

# the same library twice is one module to the loader, so the sleeper is built as two
foreach (name nsetter nsleep1 nsleep2 npinned)
    string (REGEX REPLACE "[0-9]$" "" source "${name}")
    add_library (${name} MODULE tests/modules/native/${source}.c)
    set_target_properties (${name} PROPERTIES C_STANDARD 11 C_STANDARD_REQUIRED YES PREFIX "")
    target_include_directories (${name} PRIVATE include)
endforeach ()

add_executable (phase5 tests/phase5.c)
set_target_properties (phase5 PROPERTIES C_STANDARD 11 C_STANDARD_REQUIRED YES)
target_link_libraries (phase5 PRIVATE bridge)
target_compile_definitions (phase5 PRIVATE
    "BACKEND_PATH=\"$<TARGET_FILE:bridge-wasm>\"" "NATIVE_PATH=\"$<TARGET_FILE:bridge-native>\""
    "NSETTER_PATH=\"$<TARGET_FILE:nsetter>\"" "NSLEEP1_PATH=\"$<TARGET_FILE:nsleep1>\""
    "NSLEEP2_PATH=\"$<TARGET_FILE:nsleep2>\"" "NPINNED_PATH=\"$<TARGET_FILE:npinned>\""
    "MODULE_DIR=\"${MODULES}\"")
add_dependencies (phase5 bridge-wasm bridge-native nsetter nsleep1 nsleep2 npinned c-modules-5)

function (phase5_case name expect)
    add_test (NAME phase5-${name} COMMAND phase5 ${name})
    set_tests_properties (phase5-${name} PROPERTIES PASS_REGULAR_EXPRESSION "${expect}.*alive")
endfunction ()

phase5_case (order       "sum 125250 0")
phase5_case (reorder     "sum 125500 0")
phase5_case (ambiguous   "module 'view:halver@0.1.0' did not load: 'game:mover:move' and 'view:halver:halve' both touch 'game:Position' in phase 'update', one of them writing it, and nothing orders them")
phase5_case (ownorder    "game:twostep@0.1.0/show: x 3 0")
phase5_case (cycle       ".order. contradicts itself.*bridge_create refused")
phase5_case (observe     "b2 game:setter@0.1.0/mark: marked.*b2 view:watcher@0.1.0/seen: seen 1 77")
phase5_case (chain       "b1 game:chain@0.1.0/got_a: hop A.*b2 game:chain@0.1.0/got_b: hop B.*b3 game:chain@0.1.0/got_c: hop C")
phase5_case (reserved    "'init' is a reserved name, and no observer may take it")
phase5_case (parallel    "on 2 thread.s.[^a-z]*they ran at once")
phase5_case (serial      "on 1 thread.s.[^a-z]*they ran one after the other")
phase5_case (pinned      "the others used [2-4] thread.s.[^a-z]*pinned: 9 of its 9 calls were on the thread that loaded it")
phase5_case (wrongthread "module 'test:pinned@0.1.0' is pinned to the thread that loaded it, and this call comes from another.*from another thread: refused")
phase5_case (nothreads   "threads is unset.*bridge_create refused")
# a chain that hung would log a hop twice: each is said once
set_tests_properties (phase5-chain PROPERTIES FAIL_REGULAR_EXPRESSION "hop A.*hop A")
phase5_case (hop         "sum 275000 0")
phase5_case (events      "any: event 3 1.*any: event 2 2.*any: event 4 57005")
phase5_case (obsconflict "module 'view:fixer@0.1.0' did not load: 'view:watcher:seen' and 'view:fixer:fix' both touch 'game:Marked' at the barrier, one of them writing it, and nothing orders them.*seen 1 77")
phase5_case (obsorder    "view:fixer@0.1.0: loaded.*seen 1 77")
phase5_case (obsreorder  "view:fixer@0.1.0: loaded.*seen 1 5")
# a field whose type is another declared type: schema.md, *The type set*
phase5_case (nested      "read: body 2 2.*read: more 3 4")
phase5_case (nested2     "module 'game:nested@0.1.0' did not load: component 'game:Body': its descriptor does not describe a struct, or names a type its namespace has not declared")

# a backend is handed its settings unread, and refuses one it does not know
phase4_case (setting   "backend 'native' would not start: it knows no setting 'cache'.*bridge_create refused")
# two bridges in one process, on two threads: 250000 + 500 a frame, fifty frames and thirty
phase5_case (two         "two bridges: sums 275000 and 265000")

# what a module mints is set aside for it: docs/bridge/memory/budgets.md
phase3_case (strings     "game:namer@0.1.0/say: kept in the arena")
phase3_case (noarena     "module 'game:namer@0.1.0' did not load: alloc: its persistent of 4 bytes is exhausted")
phase3_case (bigarena    "module 'game:namer@0.1.0' is refused: its persistent of 1048576 bytes does not fit in the 4096 left of .budget. persistent")
phase3_case (nospawns    "module 'game:mover@0.1.0' did not load: spawn: it stated that it spawns nothing")
phase3_case (undo        "game:wildcard@0.1.0: not loaded.*game:fp@0.1.0: loaded")
# what two modules mint in one level does not depend on which thread got there first
phase5_case (minted      "minted 160 ids a run. on four threads the same, and again the same")

# ---- phase 6: failure and lifecycle. docs/plan/PHASE6.md ----

wasm_module (flaky-always flaky checked OF=1 FAILS=1)
wasm_module (flaky-third flaky checked OF=3 FAILS=1)
wasm_module (flaky-twothirds flaky checked OF=3 FAILS=2)
wasm_module (keeper-strip keeper checked PRIVATE_POLICY=BRIDGE_STRIP)
wasm_module (keeper-sweep keeper checked PRIVATE_POLICY=BRIDGE_SWEEP)
wasm_module (keeper-delete keeper checked PRIVATE_POLICY=BRIDGE_DELETE)
wasm_module (retype-private retype checked WHICH=1)
wasm_module (retype-vec2 retype checked WHICH=2)
wasm_module (retype-extra retype checked WHICH=3)
wasm_module (retype-settings retype checked WHICH=4)
wasm_module (settings-drop settings checked DROP)
wasm_module (ticker ticker checked STEP=1)
wasm_module (ticker-ten ticker checked STEP=10)
wasm_module (ticker-wide ticker checked STEP=10 WIDE)
wasm_module (ticker-fail ticker checked STEP=10 FAIL_RELOAD)
wasm_module (ticker-noextra ticker checked STEP=10 NO_EXTRA)
wasm_module (ticker-trip ticker checked STEP=1 TRIP)
foreach (name counter typer user settings hang)
    wasm_module (${name} ${name} checked)
endforeach ()
get_property (C_MODULES GLOBAL PROPERTY C_MODULES)
add_custom_target (c-modules-6 ALL DEPENDS ${C_MODULES})

# the native ticker three ways: a library per build, since one library is one module
foreach (n 1 2 3)
    add_library (nticker${n} MODULE tests/modules/native/nticker.c)
    set_target_properties (nticker${n} PROPERTIES C_STANDARD 11 C_STANDARD_REQUIRED YES PREFIX "")
    target_include_directories (nticker${n} PRIVATE include)
endforeach ()
target_compile_definitions (nticker1 PRIVATE STEP=1)
target_compile_definitions (nticker2 PRIVATE STEP=10)
target_compile_definitions (nticker3 PRIVATE STEP=10 FAIL_RELOAD)

add_executable (phase6 tests/phase6.c)
set_target_properties (phase6 PROPERTIES C_STANDARD 11 C_STANDARD_REQUIRED YES)
target_link_libraries (phase6 PRIVATE bridge)
target_compile_definitions (phase6 PRIVATE
    "BACKEND_PATH=\"$<TARGET_FILE:bridge-wasm>\"" "NATIVE_PATH=\"$<TARGET_FILE:bridge-native>\""
    "NTICKER1_PATH=\"$<TARGET_FILE:nticker1>\"" "NTICKER2_PATH=\"$<TARGET_FILE:nticker2>\""
    "NTICKER3_PATH=\"$<TARGET_FILE:nticker3>\"" "MODULE_DIR=\"${MODULES}\"")
add_dependencies (phase6 bridge-wasm bridge-native nticker1 nticker2 nticker3 c-modules-6)

function (phase6_case name expect)
    add_test (NAME phase6-${name} COMMAND phase6 ${name})
    set_tests_properties (phase6-${name} PROPERTIES PASS_REGULAR_EXPRESSION "${expect}.*alive")
endfunction ()

phase6_case (skipinit      "game:badobs@0.1.0: not loaded.*show: x 3 0.*modules: game:twostep@0.1.0=RUNNING .1.")
phase6_case (refusedload   "game:wildcard@0.1.0: not loaded.*game:fp@0.1.0: loaded")
phase6_case (budget        "module 'test:flaky@0.1.0' is stopped: it reached its max_system_failures of 3.*show: x 63 0.*game:twostep@0.1.0=RUNNING test:flaky@0.1.0=STOPPED")
phase6_case (forgiven      "modules: test:flaky@0.1.0=RUNNING")
phase6_case (notforgiven   "it reached its max_system_failures of 3.*modules: test:flaky@0.1.0=STOPPED")
phase6_case (neverstop     "modules: test:flaky@0.1.0=RUNNING")
phase6_case (failstop      "the bridge has stopped: module 'test:flaky@0.1.0' was stopped and its on_error is fail.*update refused.*test:flaky@0.1.0=STOPPED.*game:twostep@0.1.0: not unloaded")
phase6_case (strip         "left 10 5.*game:keeper@0.1.0: unloaded.*left 9 5.*game:retype@0.1.0: loaded")
phase6_case (sweep         "left 10 5.*game:keeper@0.1.0: unloaded.*left 9 0.*game:retype@0.1.0: loaded")
phase6_case (delete        "left 10 5.*game:keeper@0.1.0: unloaded.*left 0 0.*game:retype@0.1.0: loaded")
phase6_case (nested-unload "game:typer@0.1.0: unloaded.*read: body 2 3.*game:retype@0.1.0: not loaded.*game:user@0.1.0: unloaded.*game:retype@0.1.0: loaded")
phase6_case (nounload      "no module 'game:nobody@0.1.0' is loaded, so none is unloaded")
phase6_case (timeouts      "its shutdown trapped: interrupt. its teardown continues.*test:hang@0.1.0: unloaded.*modules: .0.")
phase6_case (slots         "module 'game:ticker@0.1.0' is refused: .budget. max_modules of 1 is reached.*game:twostep@0.1.0: unloaded.*game:ticker@0.1.0: loaded.*modules: game:ticker@0.1.0=RUNNING .1.")
# the new instance's reload first, then the old one's shutdown: its line comes before the swap is said
phase6_case (reload        "tick 102 1.*game:ticker@0.1.0/-: shutdown 1 0.*game:ticker@0.2.0: reloaded.*tick 112 10.*tick 122 10")
phase6_case (reloadlayout  "was not reloaded: component 'game:Tick' has another layout here: fingerprint [0-9a-f]+, and it was declared with [0-9a-f]+.*tick 103 1.*tick 104 1")
phase6_case (reloadfail    "was not reloaded: its reload answered 5.*tick 103 1.*tick 104 1.*game:ticker@0.1.0=RUNNING")
phase6_case (reloadshare   "its persistent is -1 and a reload states 64, which only an unload and a load can change.*tick 104 1")
phase6_case (reloadmove    "it runs on backend 'wasm' and a reload does not move it to 'native'.*tick 104 1")
phase6_case (reloaddrop    "game:ticker@0.2.0: reloaded.*tick 122 10.*game:retype@0.1.0: loaded")
phase6_case (maynot        "is not reloaded: its hot_reload is false.*tick 104 1")
phase6_case (revive        "game:ticker@0.1.0=STOPPED.*game:ticker@0.2.0: reloaded.*tick 1[0-9]3 10.*game:ticker@0.2.0=RUNNING")
phase6_case (noreload      "module 'test:noreload@0.1.0' did not load: no exported function 'reload'")
phase6_case (notreloadable "module 'game:twostep@0.1.0' is refused: hot_reload is stated, and its author states it is not reloadable")
phase6_case (reloadnothing "is not reloaded: nothing of that name is loaded")
# native: the old instance's shutdown comes before the new one's first tick, and before the swap
phase6_case (nreload       "tick 102 1.*game:ticker@0.1.0/-: shutdown 1 0.*game:ticker@0.2.0: reloaded.*tick 112 10.*tick 122 10")
phase6_case (nreloadfail   "shutdown 1 0.*was not reloaded: its reload answered 5.*module 'game:ticker@0.1.0' is gone.*modules: game:twostep@0.1.0=RUNNING .1.")
phase6_case (config        "vsync width 1 1280.*scale mode 150 2.*show: hello 7 0.*show: some/long/path.cfg 0 0.*flags ratio 3 3.*show: big 2 0")
phase6_case (configkey     "its settings have a key 'vsynk', and 'game:Settings' has no field of that name")
phase6_case (configkind    "its setting 'scale' is text, and the field it names is a float")
phase6_case (configlong    "its setting 'title' is 20 bytes of text, and the field it names holds 16 with the zero that ends it")
phase6_case (configrange   "its setting 'small' is 300, which the field it names cannot hold")
phase6_case (configenum    "its setting 'mode' is 'Sometimes', which the enum of the field it names has no constant for")
phase6_case (confignone    "it is handed 10 settings and names no config component for them")
phase6_case (configmissing "its config is 'game:Nope', which is not a struct its init declared")
phase6_case (configarena   "its settings hold 24 bytes of text, and its persistent share has 8 left")
phase6_case (configtwice   "its config is 'game:Settings', which is already the settings of module 'game:settings@0.1.0'")
phase6_case (configreload  "vsync width 1 1280.*game:settings@0.2.0: reloaded.*vsync width 1 1280")
phase6_case (configdrop    "game:settings@0.2.0: reloaded.*game:retype@0.1.0: loaded")
phase6_case (reseed        "settings text written to the same place")
phase6_case (nreloadfailstop "module 'game:ticker@0.1.0' is gone.*the bridge has stopped: module 'game:ticker@0.1.0' could not be reloaded and its on_error is fail.*update refused")
phase6_case (ntimeout      "module 'game:ticker@0.1.0' is refused: shutdown_timeout needs 'deadline', which backend 'native' lacks")
set_tests_properties (phase6-ntimeout PROPERTIES FAIL_REGULAR_EXPRESSION "cannot open")
# the row a module's settings go on is an entity, under the ceiling every entity is under
add_test (NAME phase6-configceiling COMMAND phase6 configceiling 440)
set_tests_properties (phase6-configceiling PROPERTIES PASS_REGULAR_EXPRESSION
    "the row its settings go on does not fit under .budget. entities of 440.*alive")
