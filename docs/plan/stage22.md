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
test modules are written against `bridge_abi.h`, so no SDK is needed, and a case tells it which
outcome to report: a trap, a work limit used up, a timeout, a memory limit, a refusal. How it
carries a module was first ruled otherwise, and corrected: *How a test module is carried*. The
cases that prove the native backend or the wasm one go with that backend.

The library then depends on nothing outside its tree, and an outcome is had when a case asks
for it, where today some need a wasm module made to misbehave.

What it costs: the cases of the seam that hold for any backend are then in two repositories,
and may drift.

### Whose they are, and who sees them

**Public, under the licence and the notice the library has.** Enkrid ruled it on 2026-10-09:
the same as Bridge.

### What the two are called

**`BB-Native` and `BB-Wasm`, under `Encrid`.** Enkrid ruled it on 2026-10-09. BB is for a
backend of Bridge's, and what follows is the backend. It was said as "BB as a prefix", read
aloud both ways, and confirmed as the letters.

### What a test module is written with

**A binding of the tests' own, and nothing at all where a module's shape is the subject.**
Enkrid ruled it on 2026-10-09, as recommended. The binding is under `tests/`, made from the
four headers of the native backend's SDK, with names that begin `test_` and `TEST_`. It
includes `bridge_abi.h` and `bridge_schema.h` and nothing else, so the tests still show that
the seam is enough to write a module. A module whose case is about what a module is - one with
no `init`, one whose `init` has another signature - is written bare, as the fourteen in wasm
text are today.

The rule between the two: the binding, unless the binding could not say it.

It is exposed to nobody, so `tests.md` gives it no heading. What it costs: the library holds
what began as a copy of another project's SDK, and the two will differ.

### How a case gets an outcome

**The module does it, and the tests' backend judges it against what the library set.** Enkrid
ruled it on 2026-10-09, as recommended. The tests' binding gives a module four moves: to trap,
to spend so many units of work, to wait so long, to grow by so many bytes. The backend holds
each against the limit the library set on that instance through the seam and, past it, unwinds
the module and answers `call` as a backend does. No time passes: work and waiting are counted.
A case chooses an outcome by the module it loads.

So an outcome reaches the library by the seam and by nothing else, and a case that ends a
spinning system also shows that the limit it stated is the one that was set.

What a backend is, a case has by the library it names, as today: the capabilities by a setting
of the tests' backend, and the three the tests have - one with a single entry, one that answers
another number, one that refuses - built from the tests' backend where two were built from the
native one.

### Which cases stay

**Settled by the two decisions above, and by what a case proves.** No question was put: one
answer follows.

    it proves the library, and     stays, on the tests' backend. A module that was native or
    needs a module run             wasm only because it happened to be is written with the
                                   binding
    it proves what the library     stays, by the four moves. That the wasm backend produces
    does with an outcome           the outcome is a case of that backend's, written there
    it proves a backend, or an     leaves with it: the cases named for an SDK, `adopt`,
    SDK                            `nocall`, the timing of the row accessors, a module of wasm
                                   text refused for its shape
    an entry of the seam           stays, shown on the tests' backend: that the library asks
                                   it rightly. That a backend answers it rightly is that
                                   backend's
    a file that holds several      is split by function
    kinds

Each of the 484 is written down by name, with where it went, when it is moved: *How each claim
is shown*.

### The layer over the operating system

**Copied into each repository, and made its own there.** Settled by a rule that stands: a
project copies what it needs and runs nothing from another's directory. `utils/os` depends on
nothing of the library's and says so. What it costs is three copies that may differ, of 910
lines.

### What whoever writes a backend is handed

**A second package, `seams`: `bridge_backend.h`, `bridge_abi.h`, `bridge_schema.h` and
`license`.** Enkrid ruled it on 2026-10-09, as recommended. It is for whoever writes a backend,
or what a module is written with for one. A host is handed `bridge`, which keeps the one header
a host includes. Each audience takes what it uses, as the packages were cut in the first place.

A backend's repository takes both, at one commit and by the hash of each file: `seams` to be
built, and `bridge` to run a host in its own cases.

### What the packages become

**Two from here, `bridge` and `seams`, and two from each backend's repository: its own and its
SDK's.** It follows from the decision above and was not put. An SDK's package still holds
`bridge_abi.h` and `bridge_schema.h`, which its repository has from `seams`, so that whoever
writes a module takes one package.

### What a document here says of a backend's

**Nothing by name: no document of this tree cites one of a backend's.** Settled without a
question, by which way the dependence runs: a backend is written against this tree, and this
tree against no backend. A sentence here that needs what `native.md`, `wasm.md` or `wasmtime.md`
says is reworded to what the seam asks, or leaves with them. The two sections of `wasm.md`
that are of `bridge_abi.h` - its layouts, and that it holds no content - go to `abi.md`, which
is that header's document.

### What each repository is made from

**From this one: its method, its style, the tools the code is held by, and its build, each
copied and made that repository's own.** Enkrid ruled it on 2026-10-09, as recommended. The
code that leaves was written under these rules and passes these tools, so nothing is written
anew to hold it.

Each file's notice names its own project, `BB-Native Owner` and `BB-Wasm Owner`. `license` is
the same text, and `cla.md` the same agreement with the name changed. Each has a first stage of
its own that says where its content came from, by the commit of this tree.

What it costs: the method's text is then in five repositories, kept true by hand.

### How a backend takes the library

**By a pin, and from a folder where a change is being tried.** Enkrid ruled it on 2026-10-09,
as recommended. A backend's tree states one commit of this one and the hashes of `bridge` and
`seams`, in a file `bridge.txt` at its root. Its build fetches the two from the release of that
commit and refuses a file that is not what its hash says - `host_api.md`, *What a host is
handed*.

**A change to the seam goes in this order.**

    1  on a branch here. Its two packages are written into a folder, and each backend's
       build is pointed at that folder: the change is tried against both before it lands
    2  to `main` here, which publishes
    3  each backend moves its pin, passes its cases, and publishes

A package from a folder is taken as a package is, by its statement and its hashes. Nothing of
this tree is run by a backend's build.

**What a backend publishes states the commit of the library it was built against.** The number
of the seam cannot say it: nothing is released, so the number does not move while the seam
does. A host that is handed a backend built against another commit than its library refuses
it, and that check is the host's.

What it costs: three steps where there was one commit, and between 2 and 3 the newest of each
do not go together.

### What a backend publishes, and when

**Its two packages, for each commit of its `main`, by the run that tests it.** It follows from
what each repository is made from, and was not put: it is what this tree does, copied -
`host_api.md`, *What a host is handed*.

### Whether what leaves keeps its history

**It does: each repository holds the commits that touched its files.** Enkrid ruled it on
2026-10-09, over a recommendation to start each with one commit that names this tree's. So
`git log` and `git blame` answer there, for a line of a backend, as they answer here.

This tree is not written again: its history keeps the backends up to the commit that removes
them.

What it costs: commits in those repositories whose message names a stage of this plan and
whose other half is here; and the wasm backend's repository holds every build of wasmtime that
was ever in this tree.

### How a test module is carried

**As a library of its own, which the tests' backend opens by its path.** Enkrid ruled it on
2026-10-09, as recommended, in the place of what the decision on testing had first said: that
the test modules are compiled into the backend. That did not link. A module of several files
shares functions and variables between its files, and one source is built under several names:
compiled into one library, each of those names is there once for each build, and the linker
refuses. The first family moved passed only because its modules are one file each.

A library is a space of names of its own, so nothing meets, and it is how the build already
made every native module. The library still has no backend of another project and no SDK:
what opens a module's library is `utils/os`, which stays.

## The questions

None stands. Twelve were listed when the stage was planned: eight were put and ruled, and four
followed from those and from a rule that stands, each said as such under its heading.

## What was found on the way

**A module's library has its variables once in a process.** Two instances of one module share
them, where the wasm backend gives each instance its own. So a case that needs one module
twice, in two decks say, has it built under two names, as a native module was. What such a
case shows is then that the library keeps two decks apart; that two instances of one wasm
module are apart is that backend's to show. It was not put: the other way is every module's
variables moved into memory an instance is handed, in every module.

**The tests' backend stands beside the two while the cases are moved.** A deck of the tests
names three backends until the last case has left the other two.

**A module written for the wasm backend is not written again.** That backend's SDK is a
few dozen functions over a handful of accessors, which read and write at an offset into the two
memories. So the tests' binding has a second face: the SDK's headers as they were, over
accessors of the tests' own that do the same through the table. A module written by offsets
compiles as it was, but for the header it includes, and what its cases show is what they showed.
About 3,480 lines were copied and two of them changed, where a loop without end became work
and waiting that are counted.

**A module written by offsets is told its table before each call**, since its functions take
nothing. The table is kept once for each thread: a deck may have one library's module twice,
and two decks the same module, each called on a thread of its own.

**What a host hands as bytes is the path.** A module of the wasm backend is handed as bytes
and their length, and the tests' backend takes those bytes as the path of the module's
library, of the length stated and no longer. So the cases that hand bytes still show that
they arrive.

**In a deck of the tests, "wasm" and "native" are still what two backends are called**, and
both are now the tests' backend, built twice and told it keeps different things. The names
are the deployment's, and are changed when the two backends have left: until then a case's
expected text is not touched for a name.

## Where each case went

Counted against the build's list: 484 when the stage began.

    left with the wasm      `bridge_load-memory-with-a-data-segment`, and the same with the
    backend, which they     static library; `module_config.memory_limit-memory-larger-at-start`,
    prove: 9                and static; `instance_get_func-export-taking-an-argument-refused`,
                            and static; `instance_get_func-export-answering-64-bits-refused`,
                            and static; `bridge_load-import-nobody-answers`
    added: 3                `instance_get_func-no-such-export-refused`, and static: what the
                            library does when a backend finds no such function, which the
                            two that left had shown by a function of another shape.
                            `backend.settings-none-where-one-is-needed-refused`: the tests'
                            backend is not made without being told what it keeps
    reworded, the same      `BACKEND_IMPORT_OTHER-a-global-nobody-answers`: a module of the
    case: 1                 tests says it imports what nobody answers, and no instance is made

478 after the modules written for the wasm backend were moved. A case whose expected text was
the wasm backend's own words - a trap, fuel used up, an interrupt - now expects the tests'
backend's, and is the same case.

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
