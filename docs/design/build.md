# The build

What was weighed here and refused is `build_refused.md`, *What was refused: The build*.

What Bridge-ASM700 is compiled with, for what, in which C, and against which C library. None of it
is the design of the library. All of it decides whether the library can be built the same way on
three systems by somebody who installed two tools.

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

## One script

**`build.py`, and Python is the only other tool.** It compiles, links, builds the
test modules and runs the tests, as a list of commands a reader can follow from the top:

    python build.py                      build, for this machine
    python build.py test [-k NAME]       build, then run the tests
    python build.py test --copies 6      each case six times at once
    python build.py test --rounds 10     the tests ten times over
    python build.py --target T ...       build for another platform
    python build.py --debug ...          unoptimised, with debug information

CMake's work was to find and drive three compilers. With one compiler that takes its target as an
argument, what is left is running commands in order, and `tools/` was Python already.

It takes every path from where it is and from the output directory it is given, so that a host
with this repository as a submodule builds it by calling the same script.

What this gives up: the rebuild check, the parallelism and the choice of tests are ours to get
right, and an editor that reads CMake finds nothing to read.

**What it makes of the library is two files**: a shared library and a static one, from the same
objects - `host_api.md`, *A host links a built library*. The library's objects are compiled with
everything hidden but what `bridge.h` marks, so the shared library exports the host seam and
nothing more.

## Platforms

    Windows    x86-64    x86_64-windows-gnu     run where the code is written, and by a workflow
    Linux      x86-64    x86_64-linux-gnu       run by a workflow, on every push
    macOS      arm64     aarch64-macos          run by a workflow, on every push
    Linux      arm64     aarch64-linux-gnu      a target: compiled and linked, not yet run
    macOS      x86-64    x86_64-macos           a target: compiled and linked, not yet run
    Windows    arm64     aarch64-windows-gnu    a target: compiled and linked, not yet run

**A platform is one that is run.** A target is a name in `build.py`, wasmtime's library for it in
the tree, and a row of a workflow. The last three have all of that and have been compiled and
linked from the first, with what each imports held to `dependencies.md`; none has executed a case,
because no workflow has run them. Each becomes a platform, and this table says so, when its
workflow has passed.

**Windows is the `gnu` target, not `msvc`.** The `msvc` target takes its headers and libraries from
an installed Visual Studio, which is an installation *Dependencies* does not have. Both use the same
calling convention for C, so a module or a host built by either loads into the other.

**macOS is arm64 first**, the processor those machines have had since 2020 and the one the
workflow runners will keep. x86-64 is a target beside it.

**Windows on arm64 is the `gnu` target as well**, linked against the one library wasmtime
publishes for that processor, which is built by `msvc`: its import library names the functions of
a DLL, which is the same thing whichever compiler wrote it, and zig's linker reads it.

## The standard

**Each piece at its own.** wasmtime's headers are C11. Everything of Bridge-ASM700's own is C23:
the source, the tests, the test modules and every header.

    backends/wasm/wasmtime        C11    read by backends/wasm and nothing else
    everything else               C23

A file in a later standard reads a header in an earlier one, which is the direction that costs
nothing. **A standard binds only at a boundary someone else compiles**, and ours is stated here:
a host or a backend written in C, and a module in C that uses a backend's SDK, is compiled as
C23. A host need not be in C at all: it links the library built, from any language that can call
C, and for one in C++ the seam headers say they are C - `host_api.md`, *A host links a built
library*. zig is the compiler that builds all of it - *One compiler* - and a module in another
language needs none of these headers: *What an author is not bound by*.

**The language is used fully, or C23 was not worth choosing over C11.** Every older spelling that
C23 replaces is replaced, and a macro stays only where the preprocessor is what is needed:

    keywords       bool, true and false with no header; nullptr; static_assert; alignof
    enums          every one named, with its width stated: enum bridge_kind : uint32_t. A field
                   or a parameter that holds one has that type
    constants      a number or a text with a name is a typed constexpr - `style.md`, rule H3.
                   Still macros: include guards, a macro that takes arguments, and what
                   names the body of a type. An attribute takes a literal, so the one that
                   needs the namespace spells it, beside a comment that says which
                   constant it is
    attributes     [[nodiscard]] on every function whose answer is a status or something owned;
                   [[maybe_unused]] where a parameter is not read; a compiler's own in the same
                   brackets, [[clang::export_name("init")]]
    ABI layouts    static_assert of every size and offset the ABI states, in the header that
                   states it, so a module and the library cannot disagree and both compile
    checked sums   <stdckdint.h> wherever a size is computed from stated numbers. An overflow is
                   a refusal naming the key, like any other budget that cannot be met
    the rest       = {} for zero; digit separators in a large number; typeof where a macro
                   derives a type; auto only where the initialiser already names the type;
                   unreachable() after a switch that covered every case

`tools/c23.py` runs with the tests and fails on an older spelling, which is what makes "fully" a
claim that can be false. One spelling of C23 is not used: a function that takes nothing is
`f(void)`, as `style.md` rule F5 asks, and not `f()`.

**Not used, each for a reason.** `<stdbit.h>` is the C library's, and with this compiler none of the
targets has it, the six of `build.py` and wasm. `_BitInt`, `#embed` and `char8_t` have nothing
here to do.

**No variable is a thread's own.** `thread_local` is C23's and is not used: nothing in the
library or in a backend is found by which thread is running. The wasm backend finds the memory
wasmtime is asking it to create by a mark on the backend, set under its lock, because that
callback of wasmtime's carries no context - `wasmtime.md`, *Memory*. The native backend knows
which module called by the table it passes - `native.md`, *The invariant*.

## Where a function is

**What runs once for a change or for a row is `static inline` in a header, and what runs once
for a barrier or more rarely is in a `.c`.** A file holds five functions at most - `style.md`,
rule O3 - and the compiler inlines a `static` function only into a caller of the same
translation unit, so the functions a barrier's loop goes through are where every unit that
loops over them can see them. Cut into `.c` files alone, every change was measured 35 to 85
percent slower.

## The C library

A C library is three things under one name:

    pure routines     memcpy, strlen, snprintf     nothing underneath them
    allocation        malloc, free                 memory from the operating system
    the door          threads, mapping memory,     the operating system itself, on
                      loading a library            Linux and macOS

**All three are the system's.** Bridge-ASM700 is a library: it is loaded into a process whose C
library the host's executable chose, and it loads native modules that somebody else built. On
Linux a process has one C library, and a module built against another will not load into it. On
macOS the system's is the only supported way to reach the kernel. On Windows the build links
the C runtime zig carries.

**What our source calls of it is sixteen functions**, counted in the library and the two
backends: four that allocate, `memcpy`, `memmove`, `memset`, `memcmp`, `strlen`, `strcmp`,
`strncmp`, `strchr`, `snprintf`, `vsnprintf`, `strtoll` and `strtoull`.
No file, no stream, no `printf`, no floating point, no `qsort`. Each is listed as a dependency:
`dependencies.md`, *The C library*.

**The door is one layer**, `utils/os/`: mapping, threads, loading a library and the clock are
reached through it and nowhere else, and that is the only code that differs by platform.
`os.h` states it once, and a folder for each kind of platform holds the same functions:
`windows/` and `posix/`. The build takes the folder its target names, so no file of the layer
asks which platform it is compiled for. It
depends on nothing of ours, and it is compiled into the library and into each backend, which
are separate binaries.

## Dependencies

**What a build needs installed is zig and Python, and Binaryen for the wasm test modules**:
it assembles the modules written as text, and merges the SDK's accessors into a module and puts
them inline - `wasm.md`, *Memories*. **What the library
is linked against is the system's C library, and the wasm backend against wasmtime**, which is
in the tree with where it came from and its hash beside it.

Third-party code is not refused. A dependency nobody wrote down is: `dependencies.md` lists every
one with why it is there, and a case holds the list - `dependencies.md`, *Dependencies*.

## What an author is not bound by

All of the above is how Bridge-ASM700 is built. **A module is built with whatever its author has**,
in any language that can produce a wasm module or a shared library. What binds it is the ABI: the
ABI layouts of `bridge_abi.h`, the exported names, and the section its manifest sits in - `abi.md`,
*The ABI*, and `module.md`, *The manifest*. `bridge_wasm.h` and `bridge_native.h` are the two
backends' SDKs for C of that, not the contract itself, and each is its backend's: it sits in that
backend's `sdk/`, beside the backend that runs what is written with it. They are C23, and so is
whatever includes them: *The standard*.
