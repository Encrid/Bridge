# Stage 22: the backends leave the tree

Not built. `plan.md`, stage 22, is what it is for and when it is done. This document is the order
it is done in, what was counted before it began, and each decision as it is taken. What a decision
refused is `stage22_refused.md`.

It was asked for by a host that is being designed, which takes each backend built and so has to
know where one comes from. It is built here as *From here, a host's* of `plan.md` says such work
is: as a stage of this plan with its own number.

## What there was (measured on `ffda1a1`, 2026-10-09)

    the backends         `backends/native`: 7 sources, 644 lines. `backends/wasm`: 11 sources,
                         1345 lines, and wasmtime 48.0.5 in the tree, 122 files and about
                         164 MB
    their SDKs           the native one 4 headers, the wasm one 10 files: 9 headers and
                         `bridge_wasm.wat`
    shared source        `utils/os`: 13 files, 910 lines, compiled into the library and into
                         each backend. Nothing of `src/` is in a backend
    the headers they     `bridge_backend.h`, and through the SDKs `bridge_abi.h` and
    need                 `bridge_schema.h`. No package holds the first
    the cases            484: `host` 374 rows over 155 functions in 45 files, `host-static`
                         22, `storage` 54 and 5 counted, `adopt` 10, `nocall` 2, 2 built from
                         the packages, `cla` 6, and one for each of 9 tools
    the test modules     every one is written with an SDK. For wasm, about 3,480 lines of C
                         and 14 files of wasm text. Native, about 6,670 lines, most of them
                         for `tests/storage`, whose every module is native
    of the 45 files      36 prove the library alone and need something that runs a module. 9
                         are mixed. None is about one backend only
    what only wasm       12 rows: a trap, a work limit used up, a timeout, a memory limit
    answers today
    the tests' own       three, in `tests/support`. Two are the native backend with one
    backends             object replaced
    the packages         five for a target: `bridge`, `backend_native`, `backend_wasm`,
                         `sdk_native`, `sdk_wasm`. About 190 MB a release, most of it wasmtime
    tools that name      `style.py`, `c23.py`, `licence.py`, `cover.py`, `xref.py`, `said.py`,
    them                 `deps.py`, `package.py`, `nocall.py`
    the documents        three are a backend's or its SDK's: `native.md`, `wasm.md`,
                         `wasmtime.md`, cited 73 times in 33 files. Two sections of `wasm.md`
                         are of `bridge_abi.h`, and five other documents cite them

**It is small as a backend and large as a stage.** About ten thousand lines of test modules are
written with an SDK that leaves, and all of `tests/storage` runs on native modules.

## The order

    1  the stage        this document, and stage 22 in `plan.md` with a done-when
    2  the decisions    those under *The questions*, one at a time, before a file changes
    3  the tests'       built here beside the two backends. The cases are moved onto it a
       backend          family at a time, each still passing, while the two are still there
                        to compare with
    4  the two          created when Enkrid confirms each. What leaves is put there with its
       repositories     cases, and each passes against the library as it is handed
    5  this tree        the backends, the SDKs, wasmtime and the cases that left are removed,
                        and the build, the tools, the workflows and the documents say what
                        is then true
    6  the host's       where its first stage takes each backend from
       documents
    7  close            put to Enkrid by the done-when

## The decisions

### Where a backend is written

**Each of the two in a repository of its own.** Enkrid ruled it on 2026-10-09, over a
recommendation to keep both here until the seam for a backend is released. The seam stays:
`include/bridge_backend.h`, and what the documents under `seam/backend/` say of it. What leaves
is what answers it, and what a module is written with for each.

A backend runs a module's code. That is not the library's work, which is storage, the schedule
and the three seams, and it is not a host's, which orchestrates. So it is a project.

What it costs: a change to the seam is no longer one commit. It is made here, handed, and
followed in each backend's repository. And this tree no longer shows, by its own cases, that a
backend somebody uses works with the commit at hand: each backend's cases show it, after.

### How the library is tested with no backend in its tree

**By a backend of the tests' own, here.** Enkrid ruled it on 2026-10-09, as recommended. Its
test modules are compiled into it and written against `bridge_abi.h`, so no library is opened
for a module and no SDK is needed, and a case tells it which outcome to report: a trap, a work
limit used up, a timeout, a memory limit, a refusal. The cases that prove the native backend or
the wasm one go with that backend.

The library then depends on nothing outside its tree, and an outcome is had when a case asks
for it, where today some need a wasm module made to misbehave.

What it costs: the cases of the seam that hold for any backend are then in two repositories,
and may drift.

### Whose they are, and who sees them

**Public, under the licence and the notice the library has.** Enkrid ruled it on 2026-10-09:
the same as Bridge.

## The questions

Each waits here until it is put, and the stage is not done while one stands.

    1   the two names. "BB as a prefix (Bridge-Backend)" was said, and is read as
        `BB-Native` and `BB-Wasm`: to be read aloud and confirmed
    2   how a test module is written once the SDKs are gone: against `bridge_abi.h` bare,
        or with a binding kept under `tests/` for the tests alone
    3   how a case tells the tests' backend which outcome to report, and which of the
        three the tests have today become part of it
    4   which cases stay, which leave with which backend, and how a mixed file is split:
        each of the 484, by name
    5   `utils/os`, which both backends compile: copied into each repository, or handed
    6   what whoever writes a backend is handed: `bridge_backend.h`, `bridge_abi.h` and
        `bridge_schema.h` are in no package
    7   what the packages become, five being one or two, and what `host_api.md` then says
        a host takes from where
    8   where the two sections on `bridge_abi.h` go, and how a document here names one
        that is another project's
    9   what each repository is made from: its method, its style, its tools, its build,
        its licence and its agreement
    10  how each backend tests itself: against which commit of the library, taken how, and
        the order in which a change to the seam goes through three repositories
    11  whether the files that leave keep their history
    12  what each repository publishes, from which branch, and by which run

## How each claim is shown

    each case accounted for   a table here: each of the 484 by name, where it is after the
                              stage, or why it is gone, counted against the build's own list
                              before and after
    the library alone         every case, optimised and debug, alone and six at once, with no
                              `backends/` in the tree
    no name left              a search of the tree for `backends/`, `bridge_native`,
                              `bridge_wasm` and `wasmtime`: each hit is a record of a stage,
                              or a sentence on what a backend is
    the tools still fail      each shown to fail on a change that should fail it, on a copy
    each backend              its own command, on a fresh copy of its repository, with the
                              library as it is handed and nothing else of this tree
    the three together        a host built from what the three publish, run
    every other target        compiles and links
