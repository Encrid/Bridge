# Phase 8: macOS

In progress.

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

## What it builds

    third_party/wasmtime/macos/     wasmtime's library for it, as vendored for the other two
    build.py                        a third target, and one table of what differs by target
    .github/workflows/macos.yml     the tools at the pins the Linux job uses, then the tests

`src/os.c` has a POSIX half that compiles for macOS as it is. Whatever running it finds is fixed
in this phase, each with a test that fails without the fix.

## What would make us stop

- wasmtime not delivering a trap or an interrupt there under our watchdog and threads: what
  would fix it is a wasmtime setting, which is behaviour, so it is a question.
- A test that cannot pass for a reason that is the platform's: its claim would have to be
  restated, and that is not decided here. No test is skipped on macOS.
- A timing test that is unreliable on the runner: its bound is a claim, not a knob.
