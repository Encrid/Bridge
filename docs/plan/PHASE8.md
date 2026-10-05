# Phase 8: macOS

Done, on Windows, on Linux and on macOS.

A third platform, arm64, straight after the build: with one compiler that takes its target as an
argument it is one more target, and every phase after it is met on a second processor instead of
finding out last. `PLAN.md` phase 8 is the claim.

## What was settled first

    which macOS             arm64: `build.md`, *Platforms*
    when its job runs       on every push and pull request, as the Linux one does

The second was asked with its cost: the repository is private, and a macOS minute counts as ten
against what a private repository is given. Running it only when a phase is brought to merge was
the recommendation, and it lost to the rule as written: a platform is one that is run, at the
commit that could break it. So the work is ordered to push seldom, and what can be found by
compiling for macOS on Windows is found there.

Two things are the platform's and not choices. The lowest macOS is 11.0, the first that ran on
arm64 and what wasmtime's library asks for. A shared library there ends in `.dylib`.

## What it built

    third_party/wasmtime/macos/     wasmtime's library for it, as vendored for the other two
    build.py                        a third target, and one table of what differs by target
    .github/workflows/macos.yml     the tools at the pins the Linux job uses, then the tests

`build.py` had asked one question of a target, whether it was Windows, in six places. It asks
none now: a row per target says where it runs, how a program and a library are named, which of
the system's declarations are asked for by name, what a link names, how a library finds what sits
beside it, and which of wasmtime's files it uses.

Nothing in `src/`, `backends/`, `include/` or `tests/` changed.

## What it found

**Nothing, and that is the finding.** All one hundred and twenty-eight tests passed on the first
run on macOS arm64, as they had on Linux at its first run after phase 5. The POSIX half of
`src/os.c` was written against Linux and never against this system. Three things were expected to
be able to stop it and none did: wasmtime delivers a trap and an interrupt there under the
watchdog and the pool's threads, with nothing set; the timing tests hold on the runner's three
cores; and the loader takes two copies of one library as two, which the native reload tests need.

What made that possible is that the whole build was compiled and linked for macOS on Windows
before anything was pushed. One declaration macro was missing there, `_DARWIN_C_SOURCE`, as
`_DEFAULT_SOURCE` had been on Linux, and it was found by the compiler on the machine the code is
written on.

**The job costs less than was feared.** It was estimated at three minutes, thirty as a private
repository is counted. It takes under one.

## Tests

The same one hundred and twenty-eight, in a release build on macOS and on Linux, and in a release
build and a debug one on Windows. None is skipped on any of them.
