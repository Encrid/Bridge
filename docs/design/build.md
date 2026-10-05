# The build

What Bridge-ASM700 is compiled with, for what, in which C, and against which C library. None of it
is the design of the library. All of it decides whether the library can be built the same way on
three systems by somebody who installed two tools.

`PLAN.md` phases 7 to 11 build what this file states, one part each, and say which. Until a phase
is done, its part here is the decision and not yet the tree.

## One compiler

**zig, at one pinned version, compiles everything on every platform**: the library, the backends,
the tests, and the test modules, wasm and native. `zig cc` is clang with the headers and
runtimes of every target inside it, so the Linux and macOS builds are compiled where the code is
written, by the compiler that will compile them there.

It was three compilers: `cl` on Windows, gcc on Linux, and a third on its way with macOS, each at
whatever version the machine had. zig was already required, since nothing else here builds a wasm
module.

What this gives up is three opinions. `cl` and gcc disagreeing with clang found things for free,
and one compiler finds only what it finds. And zig is not at 1.0: a new version may rename a flag,
which is why the version is pinned and an upgrade is a commit of its own.

Refused: clang-cl beside `cl`, which keeps Visual Studio as something to install. Refused: each
platform's own compiler, which is where this started. The condition that brings a second compiler
back is a class of bug that only one would have caught.

## One script

**`build.py`, and Python is the only other tool.** It compiles, links, builds the
test modules and runs the tests, as a list of commands a reader can follow from the top:

    python build.py                      build, for this machine
    python build.py test [-k NAME]       build, then run the tests
    python build.py --target T ...       build for another platform
    python build.py --debug ...          unoptimised, with debug information

CMake's work was to find and drive three compilers. With one compiler that takes its target as an
argument, what is left is running commands in order, and `tools/` was Python already.

It takes every path from where it is and from the output directory it is given, so that a host
with this repository as a submodule builds it by calling the same script - `PLAN.md` phase 13.

What this gives up: the rebuild check, the parallelism and the choice of tests are ours to get
right, and an editor that reads CMake finds nothing to read. Refused: `build.zig`, which has the
cache and the targets built in, and is written in a language whose build interface changes between
releases. The condition that brings it back is zig at 1.0.

## Platforms

    Windows    x86-64    x86_64-windows-gnu     run where the code is written
    Linux      x86-64    x86_64-linux-gnu       run by a workflow, on every push
    macOS      arm64     aarch64-macos          run by a workflow, on every push

**A platform is one that is run.** wasmtime publishes its library for more than these, and zig
compiles for all of them, so a fourth is a target name, an archive and a runner. Until it has the
runner, no document claims it.

**Windows is the `gnu` target, not `msvc`.** The `msvc` target takes its headers and libraries from
an installed Visual Studio, which is the installation *Self contained* refuses. Both use the same
calling convention for C, so a module or a host built by either loads into the other.

**macOS is arm64**, the processor those machines have had since 2020 and the one the workflow
runners will keep. It is also the first build that is not x86-64, which is the reason it comes
before the phases that rewrite things. Refused: x86-64 as well, a second job and a second library
for a platform that is leaving. The condition that brings it back is a deployment on one.

## The standard

**Each piece at its own.** wasmtime's headers are C11. Everything of Bridge-ASM700's own is C23:
the source, the tests, the test modules and all five headers. flecs was C99, built by its own flags
as it asked, until phase 10 removed it.

    backends/wasm/wasmtime        C11    read by backends/wasm/wasm.c and nothing else
    everything else               C23

A file in a later standard reads a header in an earlier one, which is the direction that costs
nothing. **A standard binds only at a boundary someone else compiles**, and ours is stated here:
a host or a backend written in C, and a module in C that uses the binding headers, is compiled as
C23. zig is the compiler that builds all of it - *One compiler* - and a module in another
language needs none of these headers: *What an author is not bound by*.

**The language is used fully, or C23 was not worth choosing over C11.** Every older spelling that
C23 replaces is replaced, and a macro stays only where the preprocessor or a string literal is
what is needed:

    keywords       bool, true and false with no header; nullptr; static_assert; alignof;
                   thread_local
    enums          every one named, with its width stated: enum bridge_kind : uint32_t. A field
                   or a parameter that holds one has that type
    constants      a number with a name is a typed constexpr. Still macros: include guards, a
                   name used as a string literal or in an attribute, a macro that takes
                   arguments, a list other macros expand
    attributes     [[nodiscard]] on every function whose answer is a status or something owned;
                   [[maybe_unused]] where a parameter is not read; a compiler's own in the same
                   brackets, [[clang::export_name("init")]]
    layout         static_assert of every size and offset the ABI states, in the header that
                   states it, so a module and the library cannot disagree and both compile
    checked sums   <stdckdint.h> wherever a size is computed from stated numbers. An overflow is
                   a refusal naming the key, like any other budget that cannot be met
    the rest       = {} for zero; f() for a function that takes nothing; digit separators in a
                   large number; typeof where a macro derives a type; auto only where the
                   initialiser already names the type; unreachable() after a switch that
                   covered every case

`tools/c23.py` runs with the tests and fails on an older spelling, which is what makes "fully" a
claim that can be false.

**Not used, each for a reason.** `<stdbit.h>` is the C library's and three of the four targets
have none; *The C library* is where that could change. `_BitInt`, `#embed` and `char8_t` have
nothing here to do.

The build was C11 before anyone chose, and then C99 was chosen for the whole tree, for the sake
of one number that matched flecs. Compiling it showed the price. wasmtime's headers are not C99,
so one file needed an exception and there was no single number after all. C99 has no word for
thread-local storage, so the context would have been passed: a native module's ABI changed for
it, and the flecs patch reached into two functions that have no world to pass. Refused, then:
C99 for everything. Refused: C11, which is what the code already was and takes nothing more.
Refused: C23 for the source with the headers held to C99, which was the first recommendation: the
ABI header is where a stated width and an asserted layout are worth most. The condition that
brings an earlier standard back for the headers is a host or a backend that has to be built by a
compiler without C23.

**Thread-local storage stays.** Two variables are found by which thread is running: the native
module whose entry was called, and the memory wasmtime is being asked to create. A third went with
flecs, whose allocator callbacks could not say which bridge they were working for. Each is there
because a callback Bridge-ASM700 does not own carries no context. Phase 12 has one thing to verify
about them: on Windows the compiler reaches a thread-local variable through a runtime helper, and
that phase links no C runtime.

## The C library

A C library is three things under one name, and they do not go the same way:

    pure routines     memcpy, strlen, snprintf     nothing underneath them
    allocation        malloc, free                 memory from the operating system
    the door          threads, mapping memory,     the operating system itself, on
                      loading a library            Linux and macOS

**The first two are carried**: source in `third_party`, built by the build, linked privately into
the library. What Bridge-ASM700's own source calls is fifteen functions, and since phase 10 nothing
else in the library calls any.

**The third is not ours to carry.** Bridge-ASM700 is a library: it is loaded into a process whose C
library the host's executable chose, and it loads native modules that somebody else built. On Linux
a process has one C library, and a module built against another will not load into it. On macOS the
system's is the only supported way to reach the kernel. So `src/os.c` stays the one door, and on
those two systems the door is the system's library, for mapping, threads and `dlopen` and nothing
else. On Windows the door is `kernel32`, which is not a C library at all, and **a Windows build
links no C runtime**: a program's entry point comes from `os.c`.

Refused: a whole C library in the tree. musl is one for Linux only, and would make every native
module a musl module. Cosmopolitan covers three systems by being a way to build programs, with its
own compiler and file format. Refused: writing the routines here, a `printf` and an allocator that
C already has. The condition that brings a whole library back is native modules being required to
be built against it anyway.

One thing this buys beyond not installing anything: a record is formatted by the same code on every
platform, where three C runtimes formatted three ways.

## Self contained

**Nothing is fetched, and nothing is installed but zig and Python.** What Bridge-ASM700 depends on
is in the tree, with where it came from and its hash beside it: wasmtime today, the C
routines and the tools the build runs from phase 12.

Third-party code is not refused. Code that arrives from somewhere the tree does not record is.
That includes a C library: it has to be installed like anything else, so it counts.

## What an author is not bound by

All of the above is how Bridge-ASM700 is built. **A module is built with whatever its author has**,
in any language that can produce a wasm module or a shared library. What binds it is the ABI: the
layouts of `bridge-abi.h`, the exported names, and the section its manifest sits in - `abi.md`,
*The ABI*, and `module.md`, *The manifest*. `bridge-wasm.h` and `bridge-native.h` are C's binding
of that, not the contract itself, and each is its backend's: it sits in that backend's `sdk/`,
beside the backend that runs what is written with it. They are C23, and so is whatever includes them: *The
standard*.
