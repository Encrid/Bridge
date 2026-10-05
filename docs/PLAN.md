# Plan

The build order, and nothing else. What the design is and why is `DESIGN.md`; what the documents are
is `README.md`.

**Every phase ends in a claim that can be false.** A phase is not done because its code exists. It
is done when the stated thing is observably true. A phase whose done-when cannot fail will be
declared finished while broken.

**No done-when names something a later phase builds.** A phase is tested with what exists when it
ends, so each one carries the part of the host seam, the sink and the scheduler its own claim needs.

**Met means on every platform the phase has.** Through phase 7 that is Windows and Linux, and from
phase 8 macOS as well - `build.md`, *Platforms*. Windows is run where the code is written, and the
others by the workflows under `.github/workflows/` on every push. Phases 1 to 5 were first met on
Windows alone and said so; the Linux run caught up with them after phase 5, and passed as written.

## 0. Documents, and the spikes they need

Done. The design documents `README.md` lists were written, the register of open items was emptied
and deleted, and the documents left draft.

Two questions could not be settled by argument, so a throwaway spike settled them inside this phase:
whether flecs can be confined to our block, and whether its events are enough to keep the entity
mirror in step. Both answered with numbers, and the second answered *no* - `flecs.md`, *It can be
confined*, and *flecs will not tell you which*.

*Done when the register is empty and every design document is out of draft.* Met.

## 1. Vendor and patch flecs

Done. flecs v4.1.6, vendored pristine as the amalgamation and patched by a script: the
allocator argument at the twenty-three calls that allocate a column or an entity array, and one
row-move hook - `flecs.md`, *It can be confined*, *flecs will not tell you which*, and *How it is
carried* for the shape. The build starts in this phase. It was CMake with Ninja presets, until
phase 7 replaced it.
`PHASE1.md` is what it built and found.

**First, because everything else needs it.** Columns cannot be in the shared memory without the allocator,
and the mirror cannot be correct without the hook.

*Done when the confinement test fails if one pointer escapes the block, the row-move test fails
if one moved record goes unreported, and the patch script declines a flecs version it was not
written for.* Met.

## 2. One module runs

Done. `PHASE2.md` is what it built and found.

`bridge-backend.h` with all sixteen entries, and the wasm backend as a shared library behind it,
which Bridge-ASM700 opens by the name config gives - `backend.md`, *What a backend is asked for*. A core
wasm module importing a Bridge-ASM700 memory, which means `wasm_multi_memory` on since the module has its
own. `bridge-abi.h` begins here, because a module reading the shared memory with plain loads needs the span
layout. `define_func` with a real signature
and `instance_func`'s `() -> i32` check - `signatures.md`, *Imports are declared, exports are
checked*.

`bridge.h` begins here too, as far as this phase's claim needs: `bridge_create`, `bridge_destroy`
and `bridge_load`, with **the sink and Bridge-ASM700's own records**, so every refusal from here on has a
route - `create.md`, *The sink*. Load checks the identity and the ABI version, and each capability a
config key demands - `capabilities.md`, *Capabilities, so the swap is honest*.

What bounds a module on this backend, as far as one lifecycle call shows it: `memory_limit`, and the
init timeout - `limits.md`, *What bounds a run*.

*Done when the host reads the bytes a module wrote to the shared memory; an out-of-bounds access, a runaway
loop and growth past the limit each return an error with the process alive; and an export whose
shape is not `() -> i32`, a wrong ABI version, a backend that cannot be
opened and a null sink are each refused with a record naming the cause.* Met; the shared memory is
read by a test of the backend alone, since storage is phase 3's.

## 3. Storage

Done. `PHASE3.md` is what it built and found.

The shared memory's three regions and each module's private memory: the header, the spans,
`column_base`, the scratch arena and the log ring - `memory.md`, *Where the seven regions live*.
`[budget]`, with exhaustion fatal where it happens - `budgets.md`, *What running out does*.

`declare`, with the fingerprint, type-set and grant checks. `system`, with the query refusals -
`query.md`, *What a query may not ask for*. The structural entries `spawn`, `destroy`, `attach` and
`detach`, and `alloc`. Fuel, re-set before every dispatch. `bridge_phase` and the barrier, serial
for now: queued changes applied, spans republished, rings drained, scratch reset. The entity index
mirror, driven off the patched hook and flecs' events, and `get` over it - `reference.md`,
*Reference following*. The row-count check - `validation.md`, *Always on*. What a module compiles
against, and the module build that merges it: the `BRIDGE_SCHEMA_STRUCT` shim and accessors carrying the
`row < count` test - `schema.md`, *What a module compiles against*.

Access mode and queries are part of declaration from the start, not retrofitted.

*Done when two modules share a component array with no crossing per access, neither can reach the
other's spans, a mirror lookup survives a batch of swap-removes, and the accessor's test is measured
against a build without it* - `validation.md`, *Always on*, states that cost as reasoning, and this
is where it becomes a measurement. *And when a fingerprint mismatch, an ungranted declare and a
wildcard term are each refused by name; a system that never returns is stopped by its fuel; a
`spawn` past the entity ceiling fails where it happens; a log line reaches the sink; and an
overflowing ring reports how many it dropped.* Met.

## 4. The native backend

Done. `PHASE4.md` is what it built and found.

The second backend: a shared library that opens a module's library and calls it directly -
`native.md`. `bridge-native.h`, `bridge_bind`, and exports found by name as `bridge_<name>`. Nothing
changes above the backend seam: a native module's systems are scheduled like any other, it reads
the same spans through `private` and `shared`, and its log ring is reached through a pointer.

*Done when a native module iterates a component array through a raw pointer; a native system and a
wasm system run in the same phase with the scheduler unable to tell; and a module that
sets `trusted = false` or any limit is refused on the native backend, by the capability it
lacks, before its library is opened.* Met.

## 5. Scheduling

Done. `PHASE5.md` is what it built and found.

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

Done. `PHASE6.md` is what it built and found.

`on_error`, `max_system_failures` and the stop, and the two escalation guards - `failure.md`, *When
something fails*. `bridge_unload` and `bridge_reload` with their rules, `hot_reload`, and the reload
and shutdown timeouts - `lifetime.md`, *Lifetime*. `[.config]` seeding a singleton component -
`config.md`, *A module's own settings are a component*. `bridge_modules`, because a stop is the
state it exists to report - `introspection.md`, *Introspection*.

*Done when a skipped `init` leaves the other modules running; a refused load leaves the schedule and
storage as they were; a module past its failure budget
shows as STOPPED; a reload keeps a row's bytes; a changed layout is refused naming both
fingerprints; an unload drops an exclusive component and keeps a shared one; and a `[.config]` key
the module never declared is an error naming it.* Met.

## 7. The build

Done. One compiler and one script. zig compiles everything on every platform, and `build.py` does what
CMake did: patches flecs, compiles, links, builds the test modules and runs the tests - `build.md`,
*One compiler* and *One script*. Windows becomes `x86_64-windows-gnu`, and until phase 11 it links
the C runtime zig carries.

**Nothing about the library changes**, which is the claim: the same tests, saying the same things,
by another route. `PHASE7.md` is what it built and found.

*Done when `python build.py test` passes every test on Windows, in a release build and a debug one,
and on Linux; the Linux build compiles and links on Windows; the two tests that must fail still
fail for their stated reason; and CMake is gone from the tree.* Met.

## 8. macOS

Done. `PHASE8.md` is what it built and found.

arm64, as a third platform: a target, the POSIX half of `src/os.c` on a second system, wasmtime's
library for it and a workflow - `build.md`, *Platforms*. It comes straight after the build because
it is then one more target, and because every later phase is met on a second processor instead of
finding out last.

*Done when every test passes on macOS arm64 on every push, and that build compiles and links on
Windows.* Met.

## 9. C23, used fully

Done. `PHASE9.md` is what it built and found.

Everything of Bridge-ASM700's own becomes C23 - the source, the tests, the test modules and all
five headers - and uses it: named enums with stated widths, typed constants, `nullptr`,
attributes, every layout of the ABI asserted where it is stated, and sums of stated sizes checked.
flecs stays C99 and wasmtime's headers C11. `build.md`, *The standard*, is the rule and why.

This phase was to be C99 with the context passed in place of thread-local storage. Compiling that
before planning it showed what it cost, and the standard was chosen again: `PHASE9.md`.

*Done when everything of ours compiles as C23 with `-pedantic-errors` and warnings as errors on
every platform, and flecs as C99; `tools/c23.py` finds no older spelling; every layout the ABI
states is asserted in the header that states it; a stated size that overflows is refused by its
key; the row loop is the same code and reads the same; and every test passes on Windows, Linux
and macOS.* Met.

## 10. Storage of our own

In progress: `PHASE10.md`, *What was not in the plan*. flecs leaves, and what Bridge-ASM700 used it
for becomes its own: the entity index, tables and their columns, moving a row between them, matching
a system's terms to tables, the events an observer hears, and reading a struct's fields out of its
text. It is the reversal of a decision this plan began with, and `PHASE10.md` is why.

**flecs was never on the path that repeats.** A system reads its columns through the spans
Bridge-ASM700 publishes, and no row loop calls flecs. What flecs did, it did at a barrier and at
load. Scheduling, deferral, threads, the entity mirror and where memory sits were ours already. So
what is replaced is a hundred thousand lines kept for forty-one calls.

**It is not a smaller flecs.** flecs serves any program, and so must be right at every call.
Bridge-ASM700 touches storage only at a barrier, with every change of that barrier already in
hand, for queries fixed at load, and nobody iterates through it. Storage made for that works an
entity once however much was asked of it, publishes what changed and nothing else, believes
nothing a module can write, and allocates nothing after it is created.

**Nothing of flecs is deleted until numbers say it may be.** flecs is measured first, as it is.
Ours is built beside it, behind one internal header, and both run the same cases. flecs goes only
when ours passes all of them and is not slower on any measured operation. If it is, flecs stays
and this phase has failed its claim.

*Done when every case passes on storage that is Bridge-ASM700's own, on Windows, Linux and macOS,
alone and under load; on every measured operation it is not slower than flecs was, with both sets
of figures recorded; storage allocates nothing after `bridge_create` and reads nothing back from
the shared block; no line of flecs is left in the tree; and the behaviours no test covered before
this phase each have one.*

## 11. Self contained

Nothing fetched and nothing installed but the toolchain - `build.md`, *Self contained*. The parts
of a C library that can be carried are carried: the routines our source calls, and an
allocator, in `third_party` and linked privately - `build.md`, *The C library*. On Windows no
C runtime is linked at all. Where each routine comes from, and what "in the tree" means for the
tools the build runs, are settled in this phase's own plan, with the sources read first.

*Done when a Windows build of the library, both backends and a test program imports the operating
system's libraries and wasmtime and nothing else; the build fetches nothing; one formatted record
is the same bytes on every platform; and every test passes.*

## 12. A host, from outside

A standalone host, built in its own repository against the library and nothing else, which is what
tests the host seam: if it needs no part of Bridge-ASM700's inside, the seam is real. It has this
repository as a submodule and builds it with `build.py`. Whatever it reads - files, layers, a
manifest - it resolves into the structs of `host-api.md`, *The data*. `bridge_manifest_parse` ships
beside the library, not inside it, for a host that wants it - `native.md`, *Identity*.

*Done when that host drives a real deployment for one pass and for many; it reloads a changed module
and reports one that failed to load; a key it left unset is an error naming the key; and
`bridge_manifest_parse` reads an identity out of a wasm binary and out of a native library in each
of the three platforms' formats.*

## 13. Diagnostics

`[checks] validate` and `[checks] profile` - the barrier's checksum pass and the per-system
counters - and `bridge_profile`, a pull. `validation.md`, *The checksum pass, when asked*, and
`profile.md`, *The profile*.

*Done when a deliberate stray write is reported with the module that made it, a system made slow on
purpose is the one the profile names, an observer cycle shows as call counts that never fall to
zero, and one deployment run twice, and on one thread and on four, gives the same checksum of
every column at every barrier - `schedule.md`, *What a pass depends on*.*

## 14. Record and replay

A pass depends on storage, the phase asked for, and what native modules wrote, and on nothing
else - `schedule.md`, *What a pass depends on*. The design has paid for that since phase 0: one
path in, no deadline on a system. This phase is what it was paid for. It was listed as not in this
plan, with no condition and no phase, which is a deferral; it is planned now.

**The library's half, here.** What a recording is made of, and the two things a host needs to
make one and to play one: to be handed what native modules wrote at a barrier, and to supply a
recorded set in place of running them. No file, no format, no clock: `split.md`, *The split*. What
exactly the set is, and what a played-back run does with a native module's code, are this phase's
questions, each by the method of `PHASE0.md`.

**The host's half, in the host's repository.** The journal: its file, its format, when a run is
recorded and how one is played back. It is listed there, and it is not this repository's to build.

After diagnostics, because the checksum pass is how a replay is judged; before the tests by
capability, so that their list is whole.

*Done when a run is recorded and played back inside this repository's own tests, with a native
module that writes something different every run, and the checksum of every column at every
barrier is the same in both; a played-back run that is handed a set recorded from another
deployment is refused by name; and the standalone host records a run to a file and plays it back
from one, in its own repository.*

## 15. Tests, by capability

Every test so far was written to meet one phase's claim, and is named for the phase: `phase2`
to `phase6`. That says when a thing was built and not what is covered. This phase turns the
tests around: **one case for each capability the documents state**, named for the capability, for
the library and for the standalone host both, and a list that maps each capability to the case
that proves it. The host's half lives in the host's repository.

It comes after the last phase that adds a capability, so that the list is whole, and before the
proof, which is a workload and not a test of anything in particular.

Three things the phases did not test, and this one does. **Everything under load**: a hundred and
twenty-eight passing cases never showed the crash phase 9 found, and six copies at once showed it
in a minute - `PHASE9.md`. **Everything a document claims**: a claim with no case is found by the
list and not by reading. **Every refusal**: one, at least, existed for five phases with no test.

*Done when every capability the documents state has a case named for it, and the build fails on
one that has none; no case is named for a phase; every case passes on Windows, Linux and macOS,
alone and with several copies running at once; and the standalone host's capabilities are covered
the same way in its own repository.*

## 16. Prove it

Port the original Bridge's Godot adapter to this design, as a native module. It is a real workload
with a measured baseline, which is why it is the proof. Its probe asserts 191 checks, and two of
them ask another module a question synchronously - `costs.md`, *What we give up*, names both - so
189 is the number that can pass.

*Done when 189 of the probe's checks pass, the two that cannot are the two named, and the GDExample
frame is an order of magnitude under 42 us. If it is not, the architecture did not deliver and we
find out here.*

## Not in this plan

Each is refused or deferred in the document named, with the condition that would bring it in.

    a default phase set              `schedule.md`, *Bridge-ASM700 ships no phase names*
    bridge:clock@1 and its kind      `no-builtins.md`, *No builtins*
    per-component page mapping       `costs.md`, *What we give up*
    a phase exclusivity flag         `schedule.md`, *A phase is a name and nothing else*
    memory high-water per module     `profile.md`, *The profile*
    an instance-grouping entry       `backend.md`, *What the backend does not do*
    arena mark and reset             `strings.md`, *It is never reclaimed*
    layout migration                 `lifetime.md`, *When the layout changed*
    bindings for a non-C module      `wasm.md`, *The published layouts*
