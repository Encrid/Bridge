# Phase 7: the build

In progress.

Planning a host asked what the host would be built with, and the answer had never been chosen:
three compilers, a C standard nobody decided, a C library taken from wherever the machine had one.
Eleven questions were settled before this phase, and they are four phases of work. This is the
first: the build. `PLAN.md` phase 7 is the claim.

## What was settled first

Each by every step of `PHASE0.md`, one at a time. Most belong to later phases and are recorded
here because this is where they were asked.

    how a host gets the library   as a submodule, built by `build.py`: `PLAN.md` phase 11
    the manifest's section        `.bridge`, on every format: `native.md`, *Identity*
    macOS                         a platform, arm64: `build.md`, *Platforms*
    the standard                  C99, for everything: `build.md`, *The standard*
    the C library                 the parts that can be carried, carried: `build.md`, *The C
                                  library*
    thread-local storage          none; the context is passed: `build.md`, *No thread-local
                                  storage*
    flecs' log callback           to every live bridge's sink: the same section
    a typed accessor              through a temporary of the struct's type: `PLAN.md` phase 9
    Windows                       the `gnu` target, and no C runtime: `build.md`, *Platforms* and
                                  *The C library*
    where all this sits           a phase each, 7 to 10, before the host: `PLAN.md`

Three were not questions. The compiler is zig, everywhere; the build is a Python script; and what
all of it binds is Bridge-ASM700's own development, not a module's: `build.md`, *One compiler*,
*One script*, and *What an author is not bound by*.

The standard came out against the recommendation, which was C23 for the source with the headers
kept to C99. Three of the others follow from it.

## What it builds

`build.py` at the root, in place of `CMakeLists.txt` and `CMakePresets.json`. It does what they
did, as commands in order:

    flecs             patched three ways by `tools/patch-flecs.py`, compiled as C99
    the library       `bridge-os`, `bridge`, the two backends as shared libraries
    native modules    one shared library each
    wasm modules      assembled from text, or compiled, merged with the accessors and inlined
    the tests         one table: a name, a command, what its output must match and must not

Nothing in `src/`, `backends/` or `include/` is meant to change. This phase still compiles them as
C11, since C99 needs the changes that are phase 9.

On Windows the target is `x86_64-windows-gnu` with the C runtime zig carries, which phase 10 takes
away, and wasmtime's library is the one built for that target.

## What would make us stop

- The wasm backend not linking or loading wasmtime's `mingw` library: then the Windows target is a
  question again, before anything else is written.
- A test that passed under `cl` failing under zig for a reason in our code: a bug found, fixed
  here with its own test.
- flecs trapping in a debug build under zig's checks for undefined behaviour: a question.
