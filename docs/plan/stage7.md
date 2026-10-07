# Stage 7: the build

Done, on Windows and on Linux.

Planning a host asked what the host would be built with, and the answer had never been chosen:
three compilers, a C standard nobody decided, a C library taken from wherever the machine had one.
Eleven questions were settled before this stage, and they are four stages of work. This is the
first: the build. `plan.md` stage 7 is the claim.

## What was settled first

Each by every step of `stage0.md`, one at a time. Most belong to later stages and are recorded
here because this is where they were asked.

    how a host gets the library   as a submodule, built by `build.py`: `plan.md` stage 12
    the manifest's section        `.bridge`, on every format: `native.md`, *Identity*
    macOS                         a platform, arm64: `build.md`, *Platforms*
    the standard                  C99, for everything. Withdrawn in stage 9: `stage9.md`
    the C library                 the parts that can be carried, carried: `build.md`, *The C
                                  library*
    thread-local storage          none; the context is passed. Withdrawn in stage 9
    flecs' log callback           to every live deck's sink. Withdrawn in stage 9
    a typed accessor              through a temporary of the struct's type. Withdrawn in stage 9
    Windows                       the `gnu` target, and no C runtime: `build.md`, *Platforms* and
                                  *The C library*
    where all this sits           a stage each, 7 to 10, before the host: `plan.md`

Three were not questions. The compiler is zig, everywhere; the build is a Python script; and what
all of it binds is Bridge's own development, not a module's: `build.md`, *One compiler*,
*One script*, and *What an author is not bound by*.

The standard came out against the recommendation, which was C23 for the source with the headers
kept to C99. Three of the others follow from it.

## What it built

`build.py` at the root, in place of `CMakeLists.txt` and `CMakePresets.json`, which are gone. It
does what they did, as commands in order:

    flecs             patched three ways by `tools/patch-flecs.py`, compiled as C99
    the library       its objects, and the two backends as shared libraries
    native modules    one shared library each
    wasm modules      assembled from text, or compiled, merged with the accessors and inlined
    the tests         one table: a name, a command, what its output must match and must not

Nothing in `src/`, `backends/` or `include/` changed for it. One line of `src/store.c` and one
function of `src/os.c` did for what it found, below. They are still compiled as C11, since C99
needs the changes that are stage 9.

On Windows the target is `x86_64-windows-gnu` with the C runtime zig carries, which stage 11 takes
away, and the vendored wasmtime is the one built for that target:
`third_party/wasmtime/README.md`.

**A step runs when what it makes is missing or older than what it is made from.** That was to be
added only if zig's own cache proved too slow without it, and was written from the start instead,
since Binaryen's two passes over forty-four modules are cached by nothing. A build with nothing
changed takes 0.2 seconds, and from nothing 30.

## Tests

The same one hundred and twenty-eight, moved by a script that read them out of `CMakeLists.txt`
and checked the result against ctest's own list, name by name and in order, before that file was
deleted. In a release build and a debug one on Windows, and on Linux.

A case with a pattern passes by the pattern and not by its exit code, as before: several are a
refusal that ends the program. The runner was shown to fail on a pattern not found, on a pattern
that must not be found, and on an exit code.

## What it found

**The Linux build had been in the compiler's `gnu` mode, and nobody chose that.** CMake asks for
it unless told otherwise. In ISO mode the system's headers stop declaring `clock_gettime`,
`usleep` and `getnameinfo`, and `src/os.c`, a test module and flecs stopped compiling. The build
now asks for those declarations by name, `_DEFAULT_SOURCE`, and the language stays ISO C. This
showed on Windows, compiling for Linux, before anything was pushed.

**zig says a warning once.** A second build answers from its cache and says nothing, so a warning
is easy to never see. Our own code is compiled with warnings as errors. It had ten, all the same:
an initializer that leaves members out, in the tests and in flecs' own descriptor idiom. C makes
those members zero, and that one warning is off.

**A quiet barrier was moving 1.6 MB onto itself.** The row loop read 0.43 ns a row where it read
0.41 on the same day under `cl`, and the difference was not in the loop. Swapping the backend and
wasmtime's library between the two builds moved nothing, so it was the host's own code; timing a
phase over one row showed a fixed cost per phase; and timers around the barrier's parts put all of
it in `store_refill`. That moved the ids a module had not used to the front of its block, and when
it had used none the move was the whole block onto itself: eight bytes times its `spawns`, at
every barrier. The two C runtimes' `memmove` differ in speed, which is all the two builds
disagreed about. With nothing used, nothing is moved now: a barrier of that test went from 5.2
microseconds under `cl` and 7.7 under zig to 0.6 under both, and both read 0.39 ns a row.

**A backend's runtime was found beside the program, not beside the backend.** Windows looks for
what a library needs beside the executable, and every test passed because the build put everything
in one directory. A host in a directory of its own would have been told the backend could not be
opened. A library named by a path is now opened with its own directory searched - `create.md`,
*Create* - and the build puts the backends in a directory the test programs are not in, so that
all of them fail without it: one hundred and twenty did.

wasmtime's `mingw` library linked and loaded at the first attempt, which was the one thing that
could have sent the Windows target back to be asked again. Nothing trapped in a debug build.

## What is not done here, and where it is

Each is a later stage's claim in `plan.md`, not something left behind this one.

    macOS                             stage 8
    the standard                      stage 9
    no C runtime on Windows; nothing  stage 11
      fetched; the C routines carried
