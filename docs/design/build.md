# The build

What Bridge-ASM700 is compiled with, for what, in which C, and against which C library. None of it
is the design of the library. All of it decides whether the library can be built the same way on
three systems by somebody who installed two tools.

`PLAN.md` phases 7 to 10 build what this file states, one part each, and say which. Until a phase
is done, its part here is the decision and not yet the tree.

## One compiler

**zig, at one pinned version, compiles everything on every platform**: the library, the backends,
flecs, the tests, and the test modules, wasm and native. `zig cc` is clang with the headers and
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

**`build.py`, and Python is the only other tool.** It patches flecs, compiles, links, builds the
test modules and runs the tests, as a list of commands a reader can follow from the top:

    python build.py                      build, for this machine
    python build.py test [-k NAME]       build, then run the tests
    python build.py --target T ...       build for another platform
    python build.py --debug ...          unoptimised, with debug information

CMake's work was to find and drive three compilers. With one compiler that takes its target as an
argument, what is left is running commands in order, and `tools/` was Python already.

It takes every path from where it is and from the output directory it is given, so that a host
with this repository as a submodule builds it by calling the same script - `PLAN.md` phase 11.

What this gives up: the rebuild check, the parallelism and the choice of tests are ours to get
right, and an editor that reads CMake finds nothing to read. Refused: `build.zig`, which has the
cache and the targets built in, and is written in a language whose build interface changes between
releases. The condition that brings it back is zig at 1.0.

## Platforms

    Windows    x86-64    x86_64-windows-gnu     run where the code is written
    Linux      x86-64    x86_64-linux-gnu       run by a workflow, on every push
    macOS      arm64     aarch64-macos          run by a workflow, on every push; phase 8

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

**C99, for everything**: the source, the tests, and all five headers. flecs is C99 already and says
so. Phase 9 makes it true of the rest.

One number, and the lowest one, means any C compiler reads every header and builds every file. That
matters least for the source, which only zig compiles, and most for the headers, which a host or a
module author compiles with something else.

**It costs three things the code used**, and each has its replacement:

    _Thread_local    a context that is passed - *No thread-local storage*
    _Generic         a temporary of the struct's own type - `schema.md`, *What a module compiles
                     against*
    _Alignof         offsetof(struct { char c; T t; }, t)

Refused: C23 for the source and C99 for the headers, which takes what the compiler has and asks
nothing more of anybody else. It was the recommendation, and it lost to one number for the whole
tree. Refused: C11, the lowest standard that has all three of the above, which is what the build
used before anyone chose.

### No thread-local storage

C99 has no word for it, and the alternatives are a compiler's extension or a call into the
operating system at every read. So **nothing is found by which thread is running**. A native
module's entries take their `api` as the first argument, and each module is handed its own. The
wasm backend keeps the memory it is adopting in its own state. The flecs patch hands the allocator
the world it is allocating for.

That is the shape flecs and wasmtime both have: flecs passes a world or a stage to every call, and
every wasmtime callback takes an `env`. What it costs is the native binding, which gains an
argument at every entry, and the patch, which touches more of flecs than it did.

**One callback cannot be given a context.** flecs reports a warning or an error through a function
set once for the process, with no argument that says whose it is. So what flecs says goes to the
sink of every live bridge, marked as flecs' own. With one bridge that is exact. With two, each
hears the other's, which is noise and not an error. Refused: one thread key in `os.c` for this
alone, exact and never on a hot path, and thread-local storage all the same. The condition that
brings it back is two bridges in one process being common enough for the noise to mislead.

## The C library

A C library is three things under one name, and they do not go the same way:

    pure routines     memcpy, strlen, snprintf     nothing underneath them
    allocation        malloc, free                 memory from the operating system
    the door          threads, mapping memory,     the operating system itself, on
                      loading a library            Linux and macOS

**The first two are carried**: source in `third_party`, built by the build, linked privately into
the library, as flecs is. What Bridge-ASM700's own source calls is fifteen functions. flecs calls
more, mostly from addons that are never used, so it is built with the addons that are.

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
is in the tree, with where it came from and its hash beside it: flecs and wasmtime today, the C
routines and the tools the build runs from phase 10.

Third-party code is not refused. Code that arrives from somewhere the tree does not record is.
That includes a C library: it has to be installed like anything else, so it counts.

## What an author is not bound by

All of the above is how Bridge-ASM700 is built. **A module is built with whatever its author has**,
in any language that can produce a wasm module or a shared library. What binds it is the ABI: the
layouts of `bridge-abi.h`, the exported names, and the section its manifest sits in - `abi.md`,
*The ABI*, and `module.md`, *The manifest*. `bridge-wasm.h` and `bridge-native.h` are C's binding
of that, not the contract itself.
