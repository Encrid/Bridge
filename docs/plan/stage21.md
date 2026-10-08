# Stage 21: what a host is handed

In progress. `plan.md`, stage 21, is what it is for and when it is done. This document is the
order it is done in, what was counted before it began, and each decision as it is taken. What a
decision refused is `stage21_refused.md`.

It is the library's half of a stage of a host's plan, and is built here as *From here, a
host's* of `plan.md` says such work is: as a stage of this plan with its own number.

## What was ruled before it began

A host that is being written put four questions, and Enkrid ruled each on 2026-10-08:

    what a host is handed    units: the library, and each backend as a unit of its own
    who makes them           the library, by a step of its own build, held by a case
    how a host takes them    its build fetches them from where the library publishes them, at
                             a commit and by the hash of each file
    when the platforms run   for `main` only, and that run publishes what a host is handed

## What there was (measured on `c317353`, 2026-10-08)

    what the build makes   one folder for a target, about two hundred files: the shared
                           library (333 KB on Windows x86-64), the static one (3.1 MB), the
                           backends in a folder of their own - the native one (193 KB), the
                           wasm one, wasmtime's shared library (37 MB) and three that are the
                           tests' - and the programs and modules of the tests
    the headers            in the source tree. `bridge.h` is the one a host includes
    no step                sets apart what a host takes: `build.py` has `build` and `test`
    its own host           the program of the tests, compiled against `include/` and linked
                           with the shared library, then with the static one, inside the tree
    the workflows          three, for `dev` and pull requests since the day before. Each job
                           runs every case, then every case six at once ten times over. Five
                           jobs: Windows x86-64 in 39 minutes, Linux x86-64 in 27 and arm64 in
                           16, macOS x86-64 in 21 and arm64 in 8: about 430 counted minutes
    Windows arm64          compiled and linked from x86-64, and not run
    published              nothing: no release and no tag, and the repository is private

## The order

    1  the stage        this document, and stage 21 in `plan.md` with a done-when
    2  the decisions    nine, one at a time, before a file changes
    3  the step         the build writes the units for a target, with their statement
    4  what holds it    a case fails on a unit that is not what the document says; and the
                        program of the tests is built again from the units alone
    5  the document     `host_api.md` says what a host is handed, and where
    6  the workflows    for `main` only, and a run publishes the units of each platform
    7  the first run    the merge to `main`: what was published is fetched back and its
                        hashes checked
    8  close            put to Enkrid by the done-when

## The decisions

### What it is called

**A package.** Enkrid ruled it, as recommended. What a host takes is a package: the library's
package, a backend's package. The step of the build is `python build.py package`, and the
whole has no name of its own: it is the packages of a target. It is the ordinary word for
built files set apart for somebody else to take, and `readme.md`, *Words*, gains it.
