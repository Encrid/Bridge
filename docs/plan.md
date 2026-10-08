# Plan

The build order, and nothing else. What the design is and why is `design.md`; what the documents are
is `readme.md`.

**Every stage ends in a claim that can be false.** A stage is not done because its code exists. It
is done when the stated thing is observably true. A stage whose done-when cannot fail will be
declared finished while broken.

**No done-when names something a later stage builds.** A stage is tested with what exists when it
ends, so each one carries the part of the host seam, the sink and the scheduler its own claim needs.

**Met means on every platform the stage has.** Through stage 7 that is Windows and Linux, and from
stage 8 macOS as well - `build.md`, *Platforms*. Windows is run where the code is written, and the
others by the workflows under `.github/workflows/`, for what reaches `dev`. Stages 1 to 5
were first met on Windows alone and said so; the Linux run caught up with them after stage 5, and
passed as written.

## 0. Documents, and the spikes they need

Done. The design documents `readme.md` lists were written, the register of open items was emptied
and deleted, and the documents left draft.

Two questions could not be settled by argument, so a throwaway spike settled them inside this stage:
whether flecs can be confined to our block, and whether its events are enough to keep the entity
mirror in step. Both answered with numbers, and the second answered *no* - `flecs.md`, *It can be
confined*, and *flecs will not tell you which*.

*Done when the register is empty and every design document is out of draft.* Met.

## 1. Vendor and patch flecs

Done. flecs v4.1.6, vendored pristine as the amalgamation and patched by a script: the
allocator argument at the twenty-three calls that allocate a column or an entity array, and one
row-move hook - `flecs.md`, *It can be confined*, *flecs will not tell you which*, and *How it is
carried* for the shape. The build starts in this stage. It was CMake with Ninja presets, until
stage 7 replaced it.
`stage1.md` is what it built and found.

**First, because everything else needs it.** Columns cannot be in the shared memory without the allocator,
and the mirror cannot be correct without the hook.

*Done when the confinement test fails if one pointer escapes the block, the row-move test fails
if one moved record goes unreported, and the patch script declines a flecs version it was not
written for.* Met.

## 2. One module runs

Done. `stage2.md` is what it built and found.

`bridge_backend.h` with all sixteen entries it then had - eighteen now, `backend.md` - and the wasm
backend as a shared library behind it, which Bridge opens by the name config gives -
`backend.md`, *What a backend is asked for*. A core wasm module importing a Bridge memory,
which means `wasm_multi_memory` on since the module has its own. `bridge_abi.h` begins here, because
a module reading the shared memory with plain loads needs to know what a span holds. `define_func`
with a real signature and `instance_get_func`'s `() -> i32` check - `signatures.md`, *Imports are
declared, exports are checked*.

`bridge.h` begins here too, as far as this stage's claim needs: `bridge_create`, `bridge_destroy`
and `bridge_load`, with **the sink and Bridge's own records**, so every refusal from here on has a
route - `create.md`, *The sink*. Load checks the identity and the ABI version, and each capability a
config key demands - `capabilities.md`, *Capabilities, so the swap is honest*.

What bounds a module on this backend, as far as one lifecycle call shows it: `memory_limit`, and the
init timeout - `limits.md`, *What bounds a run*.

*Done when the host reads the bytes a module wrote to the shared memory; an out-of-bounds access, a runaway
loop and growth past the limit each return an error with the process alive; and an export whose
shape is not `() -> i32`, a wrong ABI version, a backend that cannot be
opened and a null sink are each refused with a record naming the cause.* Met; the shared memory is
read by a test of the backend alone, since storage is stage 3's.

## 3. Storage

Done. `stage3.md` is what it built and found.

The shared memory's three regions and each module's private memory: the header, the spans,
`column_base`, the scratch arena and the log ring - `memory.md`, *Where the regions live*.
`[budget]`, with exhaustion fatal where it happens - `budgets.md`, *What running out does*.

`declare`, with the fingerprint, type-set and grant checks. `add_system`, with the query refusals -
`query.md`, *What a query may not ask for*. The structural entries `spawn`, `destroy`, `attach` and
`detach`, and `alloc`. The work limit, re-set before every dispatch. `bridge_run_phase` and the barrier,
serial for now: queued changes applied, spans republished, rings drained, scratch reset. The entity index
mirror, driven off the patched hook and flecs' events, and `get` over it - `reference.md`,
*Reference following*. The row-count check, which stage 10 made needless - `validation.md`, *Always
on*. What a module compiles against, and the module build that merges it: the `ECS_STRUCT` shim, as
it was then named, and accessors carrying the `row < count` test - `schema.md`, *What a module
compiles against*.

Access mode and queries are part of declaration from the start, not retrofitted.

*Done when two modules share a component array with no crossing per access, neither can reach the
other's spans, a mirror lookup survives a batch of swap-removes, and the accessor's test is measured
against a build without it* - `validation.md`, *Always on*, states that cost as reasoning, and this
is where it becomes a measurement. *And when a fingerprint mismatch, an ungranted declare and a term
with no such operator are each refused by name; a system that never returns is stopped by its work
limit; a `spawn` past the entity ceiling fails where it happens; a log line reaches the sink; and an
overflowing ring reports how many it dropped.* Met.

## 4. The native backend

Done. `stage4.md` is what it built and found.

The second backend: a shared library that opens a module's library and calls it directly -
`native.md`. `bridge_native.h`, `bridge_bind` - gone since, for a table that is passed - and
exports found by name as `bridge_<name>`. Nothing
changes above the backend seam: a native module's systems are scheduled like any other, it reads
the same spans through `private` and `shared`, and its log ring is reached through a pointer.

*Done when a native module iterates a component array through a raw pointer; a native system and a
wasm system run in the same phase with the scheduler unable to tell; and a module that
sets `trusted = false` or any limit is refused on the native backend, by the capability it
lacks, before its library is opened.* Met.

## 5. Scheduling

Done. `stage5.md` is what it built and found.

The conflict graph, and the refusal when two conflicting systems in one phase are left unordered.
`[order]`. `observe`, with observers at the barrier, one hop
per barrier, and the reserved names refused. Threads running non-overlapping writers, which the
declared sets give us, and `pinned` - `pinned.md`, *Threads*.

*Done when a deployment orders two conflicting systems with `[order]` alone, an ambiguous pair is
named instead of guessed, a component a native module sets runs a wasm module's observer at that
same barrier, a three-deep observer chain settles in three barriers rather than hanging, and two
modules provably run in parallel. And when every system of a pinned module reports one thread id.*
Met.

## 6. Failure and lifecycle

Done. `stage6.md` is what it built and found.

`on_error`, `max_system_failures` and the stop, and the two escalation guards - `failure.md`, *When
something fails*. `bridge_unload` and `bridge_reload` with their rules, `hot_reload`, and the reload
and shutdown timeouts - `lifetime.md`, *Lifetime*. `[.config]` seeding a singleton component -
`config.md`, *A module's own settings are a component*. `bridge_list_modules`, because a stop is the
state it exists to report - `introspection.md`, *Introspection*.

*Done when a skipped `init` leaves the other modules running; a refused load leaves the schedule and
storage as they were; a module past its failure budget
shows as STOPPED; a reload keeps a row's bytes; a changed component layout is refused naming both
fingerprints; an unload drops an exclusive component and keeps a shared one; and a `[.config]` key
the module never declared is an error naming it.* Met.

## 7. The build

Done. One compiler and one script. zig compiles everything on every platform, and `build.py` does what
CMake did: patches flecs, compiles, links, builds the test modules and runs the tests - `build.md`,
*One compiler* and *One script*. Windows becomes `x86_64-windows-gnu`, and it links the C
runtime zig carries.

**Nothing about the library changes**, which is the claim: the same tests, saying the same things,
by another route. `stage7.md` is what it built and found.

*Done when `python build.py test` passes every test on Windows, in a release build and a debug one,
and on Linux; the Linux build compiles and links on Windows; the two tests that must fail still
fail for their stated reason; and CMake is gone from the tree.* Met.

## 8. macOS

Done. `stage8.md` is what it built and found.

arm64, as a third platform: a target, the POSIX half of the operating system's layer on a second
system, wasmtime's library for it and a workflow - `build.md`, *Platforms*. It comes straight
after the build because it is then one more target, and because every later stage is met on a
second processor instead of finding out last.

*Done when every test passes on macOS arm64 on every push, and that build compiles and links on
Windows.* Met.

## 9. C23, used fully

Done. `stage9.md` is what it built and found.

Everything of Bridge's own becomes C23 - the source, the tests, the test modules and all
five headers - and uses it: named enums with stated widths, typed constants, `nullptr`,
attributes, every ABI layout asserted where it is stated, and sums of stated sizes checked.
flecs stays C99 and wasmtime's headers C11. `build.md`, *The standard*, is the rule and why.

This stage was to be C99 with the context passed in place of thread-local storage. Compiling that
before planning it showed what it cost, and the standard was chosen again: `stage9.md`.

*Done when everything of ours compiles as C23 with `-pedantic-errors` and warnings as errors on
every platform, and flecs as C99; `tools/c23.py` finds no older spelling; every ABI layout the header
states is asserted in the header that states it; a stated size that overflows is refused by its
key; the row loop is the same code and reads the same; and every test passes on Windows, Linux
and macOS.* Met.

## 10. Storage of our own

Done. `stage10.md` is what it built and found. flecs leaves, and what Bridge used it
for becomes its own: the entity index, tables and their columns, moving a row between them, matching
a system's terms to tables, the events an observer hears, and reading a struct's fields out of its
text. It is the reversal of a decision this plan began with, and `stage10.md` is why.

**flecs was never on the path that repeats.** A system reads its columns through the spans
Bridge publishes, and no row loop calls flecs. What flecs did, it did at a barrier and at
load. Scheduling, deferral, threads, the entity mirror and where memory sits were ours already. So
what is replaced is a hundred thousand lines kept for forty-one calls.

**It is not a smaller flecs.** flecs serves any program, and so must be right at every call.
Bridge touches storage only at a barrier, with every change of that barrier already in
hand, for queries fixed at load, and nobody iterates through it. Storage made for that works an
entity once however much was asked of it, publishes what changed and nothing else, believes
nothing a module can write, and allocates nothing after it is created.

**Nothing of flecs is deleted until numbers say it may be.** flecs is measured first, as it is.
Ours is built beside it, behind one internal header, and both run the same cases. flecs goes only
when ours passes all of them and is not slower on any measured operation. If it is, flecs stays
and this stage has failed its claim.

*Done when every case passes on storage that is Bridge's own, on Windows, Linux and macOS,
alone and under load; on every measured operation it is not slower than flecs was, with both sets
of figures recorded; storage allocates nothing after `bridge_create` and reads nothing back from
the shared block; no line of flecs is left in the tree; and the behaviours no test covered before
this stage each have one.* Met.

## 11. Storage layouts

Done. `stage11.md` is what it weighed, decided, built and found.

Storage keeps every component one way: in the table of its entity's archetype, a column beside
the entity's other components. That is the right way for what systems read together, and one
way for everything is one trade made for everything. It was never chosen: it was what flecs did
by default, and what was meant from the start was the right tool for each task. The line that
carried the intent, "layout migration", was read for six stages as something else -
`stage10.md`, *The repair*.

**A component is kept in the storage layout stated for it.** Five, by the names the projects
that have them use:

    table             the archetype's table, a column a component. What systems read
                      together, row by row. What exists
    sparse set        the component's own packed array, and an index by entity. What comes and
                      goes, what few entities carry, what sits on entities that carry a lot
    toggle            a bit a row that turns a component on or off where it is. What an entity
                      always has and sometimes does not use
    direct-indexed    the value of entity i at place i. What nearly every entity carries and is
                      reached by id far more than it is walked
    singleton         one value for the deployment, at one place. Settings, and what there is
                      one of

They do not overlap: each is weak where another is strong, and a component stored the wrong way
pays that storage layout's price. `storage weigh` measures the price of the one there is: a component
given to an entity that carries 4 KB costs thirteen times what it costs a small one, and the
same rows read from 4,096 tables take three times as long as from one.

**Each of the four that do not exist is weighed before it is built, and one that fails is not
built.** Only the sparse set answers something measured. The other three were accepted on an
argument: what a toggle saves over a move between tables, what direct-indexed saves over a sparse
set's index and what it costs in room for entities that do not carry the component, and what a
singleton is that a component on one entity is not, which is how a module's settings are kept
today. So for each, first: the weakness it answers, measured in scratch beside the layouts that
exist; then all six needs of `contributing.md`, step 6, scored and shown. That comes before this
stage's other questions.

This stage's questions, each by the method of `contributing.md`, and in this order: who states a
component's storage layout, and where; what a module is handed for each, in `bridge_abi.h` and in
each backend's SDK, and what a query may ask across two of them; what each costs of which
budget; whether a component may have no bytes at all, a tag; and what it takes to move a
component from one storage layout to another, which is what "layout migration" meant.

**And a type's rows are moved to a new component layout here too**, because it is the same work:
writing every row of a type again somewhere else. The rules are decided - `lifetime.md`, *When a
component layout changed* - and until this stage a changed component layout is refused. One
question is left to this stage: how a reload states that it accepts a loss of values.

Before everything that reads storage or describes it: the readings of how full a budget is, and
the module seam in a second language, would each be made twice otherwise.

*Done when each of the five has been weighed on every need and either passed or left the plan with
its condition; a component can be declared in each that passed and a module reads it as that storage
layout is read; each has a case for what it is for and a measurement beside the table's for the same
work, and none is slower than the table at what it was chosen for; a query across two storage
layouts either works as written or is refused by name; running out, strip at unload and what an
observer is told each hold in every storage layout; a component is moved from one storage layout to
another with its values; and a module reloaded with a field added, removed, renamed or moved keeps
every value its new component layout still has a place for, in each storage layout; and a reload
that would drop values is refused, naming them, until it states that it accepts the loss, and is
then told by a record what went.* Met.

## 12. One style, every platform, few dependencies

Built, with one clause of its done-when waiting on a thing outside the tree: `stage12.md`,
*The close*. Three things that are about the whole tree and not about what it does, done before
a host is written against it, so that it is written against names that stay.

**One coding style, and a program that holds it.** `style.md` is how the C is written, and
every C file of ours follows it: the library, the seam headers, the backends and their SDKs,
the test programs and the test modules. Its numbers are hard: 80 columns, 20 lines a function,
5 functions a file, 4 arguments. An infringement is an entry of its *Exceptions*, with its
reason, or it is a defect, and `tools/style.py` runs with the tests and says which. The seams
follow it as everything else does: a type's name ends in `_t` in the public headers too, and
an entry takes four arguments. And where the style asks for a logical entity a file, the tree
is cut that way: storage by what keeps a component, the operating system once and not three
times, a test program as a table of its cases.

**Every platform run, and more of them.** A platform is one that a workflow runs on every
push - `build.md`, *Platforms* - and Windows had none. Linux on arm64, macOS on x86-64 and
Windows on arm64 are added, each with a target, wasmtime's library for it and a runner.

**Few dependencies, and every one written down.** What a build needs installed and what the
library is linked against are listed in one place with why each is there, and a case fails on
one that is not listed. Then the list is shortened where it can be: the tools the test modules
are built with, and the C library's functions the library calls.

**No refactor commit goes unmeasured.** A style of short functions in many files is the shape
that cost a barrier 4 to 6 ns in stage 11, when the compiler stopped putting its helpers
inline. `storage measure`, `storage weigh` and the stress run are run beside the tree before
this stage at every commit that rewrites code, and a slower line stops the commit.

This stage was to be "self contained": the C library's routines and an allocator carried in the
tree, and no C runtime linked on Windows. That is not built, and why is the record of
`build.md`, *The C library*.

*Done when `tools/style.py` finds no breach in any C file of ours outside the exceptions
`style.md` lists; every case passes on every platform the stage has, each run by a workflow on
every push; no line of `storage measure`, `storage weigh` or the measurements beside flecs and
EnTT is slower than before the stage by more than aligning every function moves it, and each
that is slower at all is on record with what was tried; and every dependency is in the list,
with a case that fails on one that is not.*

**The order from here was changed on 2026-10-07, at Enkrid's word.** The library is finished
first: its tests, its documents, its name. The host comes after, with the two stages that belong
to it, diagnostics and the seam for a language that is not C. It replaces an order in which the
host was next, stage 13, because a host from outside is what tests the host seam; that reason
still holds, and what it costs to wait is said in *From here, a host's*.

## 13. Tests, by capability

Built, with one clause of its done-when waiting on a thing outside the tree: `stage13.md`, *The
close*.

Every test so far was written to meet one stage's claim, and is named for the stage: `stage2`
to `stage6`. That says when a thing was built and not what is covered. This stage turns the
tests around: **one case for each capability**, named for the capability, and a list that maps
each capability to the case that proves it. A capability is what the library exposes on one of
its three seams, a thing somebody outside it can interact with: `stage13.md`, *What a capability
is*. This is the library's half. The standalone host's is the host's, in its own repository.

It was to come after the last stage that adds a capability, so that the list would be whole. It
comes before work that adds some, *From here, a host's*, and so it states the rule that keeps the list
whole from here on: **a capability is added with its case**, named for the capability and in the
list, and the build fails on one that has none.

Three things the stages did not test, and this one does. **Everything under load**: a hundred and
twenty-eight passing cases never showed the crash stage 9 found, and six copies at once showed it
in a minute - `stage9.md`. **Everything that is exposed**: a name with no case is found by the
list and not by reading. **Every refusal**: one, at least, existed for five stages with no test.

*Done when every capability has a case named for it, and the build fails on one that has none;
no case is named for a stage; every case passes on Windows, Linux and macOS, alone and with
several copies running at once.*

## 14. The documents

Built: `stage14.md`.

The documents were written a stage at a time, each to say what that stage decided. Before the
library is named and a host is written against it, they are read as one thing. What of them is to
be improved - what a reader who was not there cannot follow, what two documents say twice, what a
case of stage 13 showed to be claimed and not held - is this stage's first question, by the method
of `contributing.md`. One thing is its already: each name a header exposes cites, in the comment above
it, the document that states it, and the build fails on one that cites none - `stage13.md`,
*Where a name's document is said*. It comes after the tests by capability, whose list is what
finds a claim with no case, and before the rename, so that what is renamed is settled.

*Done when each thing this stage's first question listed is changed or refused on record; `xref.py`
finds no broken citation; and every case passes.*

## 15. Bridge

Built: `stage15.md`.

The library is finished as a library, and takes the name of what it replaces: the name it was
built under goes. What is renamed - the repository, the name in every document and in the
header every file of ours starts with, and what the design before it is called from then on in
the documents that refer to it - is this stage's first question, by the method of `contributing.md`.
`stage15.md` says what each was called.

*Done when nothing in the tree says the name the library was built under, but the record of
this stage; the design before it has one name wherever it is meant; and every case passes.*

## From here, a host's

**Four stages followed, and on 2026-10-07 Enkrid moved them to the plan of a host, in the host's
own repository.** The library is done for now, and each of the four is led by what a host needs.
A record or a refusal in this tree that says one of these numbers means the stage it was:

    16  A host, from outside        a standalone host that links the built library and is
                                    handed nothing else of it, in C and in C++ at least
    17  Diagnostics                 the checksum pass, the profile, and how full each budget
                                    is: `validation.md` and `profile.md` say each is not built
    18  A seam, for a language      the three seam headers in a form that needs no C parser,
        that is not C               and one module written in a language that is not C
    19  Prove it                    a real workload ported and measured: what a work limit
                                    costs it, how idle the other threads are beside its
                                    heaviest system, and whether deployments share a clock
                                    and a list of phase names

**What one of them asks of the library is built here**, in this tree, when that plan reaches it:
by the method of `contributing.md`, as a stage of this plan with its own number, and with the case
that the rule of stage 13 demands for whatever it exposes.

**What it costs.** The host seam is called finished before anything but our own tests has linked
it, and a host from outside is what tests it. What the first host finds wrong with the seam is
changed here.

## 20. The licence

Built, with four things not yet shown: `stage20.md`, *The close*.

Bridge has no licence, and every file of ours says that all rights are reserved. The design
before it had one that was its own: the GNU General Public License, with an exception by which
what it loads is not a derivative of it, and an agreement that lets the whole be offered where
that licence cannot go. That text was written for a program nothing linked. This is a library
that a host links, whose seam headers are compiled into other people's work, and none of the
paths it names is here. What is wanted of it, and what the text has to say to mean that, is this
stage's questions, by the method of `contributing.md`. Nobody working on it is a lawyer, and it
says where that matters.

*Done when the licence is in the tree, and is the published text, byte for byte; no file of
ours says all rights are reserved, and `style` fails on one that does; each thing in the tree
that is somebody else's is named with its licence; a document says, once, what a host, a module
and a backend may be under, and where other terms are had; and every case passes.*

## 21. What a host is handed

In progress: `stage21.md`.

A host links the built library and does not compile it - `host_api.md`, *A host links a built
library*. Nothing says what that host is given, or where it finds it: the build writes two
hundred files into one folder, and the header is in the source tree. This stage sets apart
what a host takes, as packages - the library's, and for each backend its own and its SDK's -
writes them
by a step of the build, says in each which commit it is and the hash of every file, and
publishes them from `main`, for each platform, by the run that tests it. The platforms run for
`main` only from this stage on.

*Done when one command writes, for a target, the library's package and, for each backend, its own
and its SDK's, each holding what `host_api.md` says it holds and a statement of the commit and of
the hash of each file, and a case fails on a package that differs; a host, and a module for each
backend, are built from the packages alone and run; and a run for `main` publishes the packages of
each platform, from which one is fetched by its commit and its hashes hold.*
