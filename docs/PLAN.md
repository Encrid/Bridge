# Plan

The build order, and nothing else. What the design is and why is `DESIGN.md`; what the documents are
is `README.md`.

**Every stage ends in a claim that can be false.** A stage is not done because its code exists. It
is done when the stated thing is observably true. A stage whose done-when cannot fail will be
declared finished while broken.

**No done-when names something a later stage builds.** A stage is tested with what exists when it
ends, so each one carries the part of the host seam, the sink and the scheduler its own claim needs.

**Met means on every platform the stage has.** Through stage 7 that is Windows and Linux, and from
stage 8 macOS as well - `build.md`, *Platforms*. Windows is run where the code is written, and the
others by the workflows under `.github/workflows/` on every push. Stages 1 to 5 were first met on
Windows alone and said so; the Linux run caught up with them after stage 5, and passed as written.

## 0. Documents, and the spikes they need

Done. The design documents `README.md` lists were written, the register of open items was emptied
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
`STAGE1.md` is what it built and found.

**First, because everything else needs it.** Columns cannot be in the shared memory without the allocator,
and the mirror cannot be correct without the hook.

*Done when the confinement test fails if one pointer escapes the block, the row-move test fails
if one moved record goes unreported, and the patch script declines a flecs version it was not
written for.* Met.

## 2. One module runs

Done. `STAGE2.md` is what it built and found.

`bridge-backend.h` with all sixteen entries, and the wasm backend as a shared library behind it,
which Bridge-ASM700 opens by the name config gives - `backend.md`, *What a backend is asked for*. A core
wasm module importing a Bridge-ASM700 memory, which means `wasm_multi_memory` on since the module has its
own. `bridge-abi.h` begins here, because a module reading the shared memory with plain loads needs to know
what a span holds. `define_func` with a real signature
and `instance_func`'s `() -> i32` check - `signatures.md`, *Imports are declared, exports are
checked*.

`bridge.h` begins here too, as far as this stage's claim needs: `bridge_create`, `bridge_destroy`
and `bridge_load`, with **the sink and Bridge-ASM700's own records**, so every refusal from here on has a
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

Done. `STAGE3.md` is what it built and found.

The shared memory's three regions and each module's private memory: the header, the spans,
`column_base`, the scratch arena and the log ring - `memory.md`, *Where the seven regions live*.
`[budget]`, with exhaustion fatal where it happens - `budgets.md`, *What running out does*.

`declare`, with the fingerprint, type-set and grant checks. `system`, with the query refusals -
`query.md`, *What a query may not ask for*. The structural entries `spawn`, `destroy`, `attach` and
`detach`, and `alloc`. Fuel, re-set before every dispatch. `bridge_phase` and the barrier, serial
for now: queued changes applied, spans republished, rings drained, scratch reset. The entity index
mirror, driven off the patched hook and flecs' events, and `get` over it - `reference.md`,
*Reference following*. The row-count check, which stage 10 made needless - `validation.md`, *Always
on*. What a module compiles against, and the module build that merges it: the `ECS_STRUCT` shim, as
it was then named, and accessors carrying the `row < count` test - `schema.md`, *What a module
compiles against*.

Access mode and queries are part of declaration from the start, not retrofitted.

*Done when two modules share a component array with no crossing per access, neither can reach the
other's spans, a mirror lookup survives a batch of swap-removes, and the accessor's test is measured
against a build without it* - `validation.md`, *Always on*, states that cost as reasoning, and this
is where it becomes a measurement. *And when a fingerprint mismatch, an ungranted declare and a
wildcard term are each refused by name; a system that never returns is stopped by its fuel; a
`spawn` past the entity ceiling fails where it happens; a log line reaches the sink; and an
overflowing ring reports how many it dropped.* Met.

## 4. The native backend

Done. `STAGE4.md` is what it built and found.

The second backend: a shared library that opens a module's library and calls it directly -
`native.md`. `bridge-native.h`, `bridge_bind`, and exports found by name as `bridge_<name>`. Nothing
changes above the backend seam: a native module's systems are scheduled like any other, it reads
the same spans through `private` and `shared`, and its log ring is reached through a pointer.

*Done when a native module iterates a component array through a raw pointer; a native system and a
wasm system run in the same phase with the scheduler unable to tell; and a module that
sets `trusted = false` or any limit is refused on the native backend, by the capability it
lacks, before its library is opened.* Met.

## 5. Scheduling

Done. `STAGE5.md` is what it built and found.

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

Done. `STAGE6.md` is what it built and found.

`on_error`, `max_system_failures` and the stop, and the two escalation guards - `failure.md`, *When
something fails*. `bridge_unload` and `bridge_reload` with their rules, `hot_reload`, and the reload
and shutdown timeouts - `lifetime.md`, *Lifetime*. `[.config]` seeding a singleton component -
`config.md`, *A module's own settings are a component*. `bridge_modules`, because a stop is the
state it exists to report - `introspection.md`, *Introspection*.

*Done when a skipped `init` leaves the other modules running; a refused load leaves the schedule and
storage as they were; a module past its failure budget
shows as STOPPED; a reload keeps a row's bytes; a changed component layout is refused naming both
fingerprints; an unload drops an exclusive component and keeps a shared one; and a `[.config]` key
the module never declared is an error naming it.* Met.

## 7. The build

Done. One compiler and one script. zig compiles everything on every platform, and `build.py` does what
CMake did: patches flecs, compiles, links, builds the test modules and runs the tests - `build.md`,
*One compiler* and *One script*. Windows becomes `x86_64-windows-gnu`, and until stage 12 it links
the C runtime zig carries.

**Nothing about the library changes**, which is the claim: the same tests, saying the same things,
by another route. `STAGE7.md` is what it built and found.

*Done when `python build.py test` passes every test on Windows, in a release build and a debug one,
and on Linux; the Linux build compiles and links on Windows; the two tests that must fail still
fail for their stated reason; and CMake is gone from the tree.* Met.

## 8. macOS

Done. `STAGE8.md` is what it built and found.

arm64, as a third platform: a target, the POSIX half of `src/os.c` on a second system, wasmtime's
library for it and a workflow - `build.md`, *Platforms*. It comes straight after the build because
it is then one more target, and because every later stage is met on a second processor instead of
finding out last.

*Done when every test passes on macOS arm64 on every push, and that build compiles and links on
Windows.* Met.

## 9. C23, used fully

Done. `STAGE9.md` is what it built and found.

Everything of Bridge-ASM700's own becomes C23 - the source, the tests, the test modules and all
five headers - and uses it: named enums with stated widths, typed constants, `nullptr`,
attributes, every ABI layout asserted where it is stated, and sums of stated sizes checked.
flecs stays C99 and wasmtime's headers C11. `build.md`, *The standard*, is the rule and why.

This stage was to be C99 with the context passed in place of thread-local storage. Compiling that
before planning it showed what it cost, and the standard was chosen again: `STAGE9.md`.

*Done when everything of ours compiles as C23 with `-pedantic-errors` and warnings as errors on
every platform, and flecs as C99; `tools/c23.py` finds no older spelling; every ABI layout the header
states is asserted in the header that states it; a stated size that overflows is refused by its
key; the row loop is the same code and reads the same; and every test passes on Windows, Linux
and macOS.* Met.

## 10. Storage of our own

In progress: `STAGE10.md`, *What was not in the plan*. flecs leaves, and what Bridge-ASM700 used it
for becomes its own: the entity index, tables and their columns, moving a row between them, matching
a system's terms to tables, the events an observer hears, and reading a struct's fields out of its
text. It is the reversal of a decision this plan began with, and `STAGE10.md` is why.

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
and this stage has failed its claim.

*Done when every case passes on storage that is Bridge-ASM700's own, on Windows, Linux and macOS,
alone and under load; on every measured operation it is not slower than flecs was, with both sets
of figures recorded; storage allocates nothing after `bridge_create` and reads nothing back from
the shared block; no line of flecs is left in the tree; and the behaviours no test covered before
this stage each have one.*

## 11. Storage layouts

Storage keeps every component one way: in the table of its entity's archetype, a column beside
the entity's other components. That is the right way for what systems read together, and one
way for everything is one trade made for everything. It was never chosen: it was what flecs did
by default, and what was meant from the start was the right tool for each task. The line that
carried the intent, "layout migration", was read for six stages as something else -
`STAGE10.md`, *The repair*.

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

This stage's questions, each by the method of `STAGE0.md`, and in this order: who states a
component's storage layout, and where; what a module is handed for each, in `bridge-abi.h` and in
each backend's SDK, and what a query may ask across two of them; what each costs of which
budget; whether a component may have no bytes at all, a tag; and what it takes to move a
component from one storage layout to another, which is what "layout migration" meant.

**And a type's rows are moved to a new component layout here too**, because it is the same work:
writing every row of a type again somewhere else. The rules are decided - `lifetime.md`, *When a
component layout changed* - and until this stage a changed component layout is refused.

Before everything that reads storage or describes it: the readings of how full a budget is, and
the module seam in a second language, would each be made twice otherwise.

*Done when a component can be declared in each of the five and a module reads it as that storage
layout is read; each has a case for what it is for and a measurement beside the table's for the same
work, and none is slower than the table at what it was chosen for; a query across two storage
layouts either works as written or is refused by name; running out, strip at unload and what an
observer is told each hold in every storage layout; a component is moved from one storage layout
to another with its values; and a module reloaded with a field added, removed, renamed or moved
keeps every value its new component layout still has a place for, in each storage layout, and is
told by a record what went.*

## 12. Self contained

Nothing fetched and nothing installed but the toolchain - `build.md`, *Self contained*. The parts
of a C library that can be carried are carried: the routines our source calls, and an
allocator, in `third_party` and linked privately - `build.md`, *The C library*. On Windows no
C runtime is linked at all. Where each routine comes from, and what "in the tree" means for the
tools the build runs, are settled in this stage's own plan, with the sources read first.

*Done when a Windows build of the library, both backends and a test program imports the operating
system's libraries and wasmtime and nothing else; the build fetches nothing; one formatted record
is the same bytes on every platform; and every test passes.*

## 13. A host, from outside

A standalone host, built in its own repository against the library and nothing else, which is what
tests the host seam: if it needs no part of Bridge-ASM700's inside, the seam is real. It has this
repository as a submodule and builds it with `build.py`. Whatever it reads - files, layers, a
manifest - it resolves into the structs of `host-api.md`, *The data*. `bridge_manifest_parse` ships
beside the library, not inside it, for a host that wants it - `native.md`, *Identity*.

*Done when that host drives a real deployment for one pass and for many; it reloads a changed module
and reports one that failed to load; a key it left unset is an error naming the key; and
`bridge_manifest_parse` reads an identity out of a wasm binary and out of a native library in each
of the three platforms' formats.*

## 14. Diagnostics

`[checks] validate` and `[checks] profile` - the barrier's checksum pass and the per-system
counters - and `bridge_profile`, a pull. `validation.md`, *The checksum pass, when asked*, and
`profile.md`, *The profile*. And a second pull: how much of each budget is in use, and the most
that ever was, for the bridge and for each module - the reading a deployment sizes its budgets by.
What a host does with it is the host's, in its own repository.

*Done when a deliberate stray write is reported with the module that made it, a system made slow on
purpose is the one the profile names, an observer cycle shows as call counts that never fall to
zero, every budget of the bridge and of a module reads as what a test made it use and as the
most it ever used, and one deployment run twice, and on one thread and on four, gives the same checksum of
every column at every barrier - `schedule.md`, *What a pass depends on*.*

## 15. A module that is not C

What a module needs to be written falls in two parts, and they have two owners - `README.md`,
*Three layers, three seams*.

    Bridge-ASM700     what it writes for a module to read, the same on every backend:
                      `bridge-abi.h`
    a backend         how a module it runs reaches that: its SDK, in its own `sdk/`

Both are C23 today, and `wasm.md`, *The ABI layouts*, said a module in another language
derives what it needs from the header. That was never tried, and the first try failed: the
project's own compiler cannot translate `bridge-abi.h`, which stops at its first `constexpr`. So
a module in any language but C is a claim. It was listed as not in this plan, with no condition
and no stage.

This stage makes it true once, for one language that is not C, on the native backend: the ABI
layouts in a form that language can read, and that backend's SDK in that language, each made by its
owner. How Bridge-ASM700 states its ABI layouts so that more than C can read them - a description
with no language in it from which the header is written, or something less - is this stage's
first question, by the method of `STAGE0.md`; which language, and what a backend's SDK in it
consists of, are the next two.

After diagnostics; and before the tests by capability, whose list this adds to.

*Done when a native module in a language that is not C runs as a case; what it was written with
came from Bridge-ASM700 for the ABI layouts and from the native backend for the rest, and neither was
copied out of the other; and an ABI layout changed in one place changes what both languages read.*

## 16. Tests, by capability

Every test so far was written to meet one stage's claim, and is named for the stage: `stage2`
to `stage6`. That says when a thing was built and not what is covered. This stage turns the
tests around: **one case for each capability the documents state**, named for the capability, for
the library and for the standalone host both, and a list that maps each capability to the case
that proves it. The host's half lives in the host's repository.

It comes after the last stage that adds a capability, so that the list is whole, and before the
proof, which is a workload and not a test of anything in particular.

Three things the stages did not test, and this one does. **Everything under load**: a hundred and
twenty-eight passing cases never showed the crash stage 9 found, and six copies at once showed it
in a minute - `STAGE9.md`. **Everything a document claims**: a claim with no case is found by the
list and not by reading. **Every refusal**: one, at least, existed for five stages with no test.

*Done when every capability the documents state has a case named for it, and the build fails on
one that has none; no case is named for a stage; every case passes on Windows, Linux and macOS,
alone and with several copies running at once; and the standalone host's capabilities are covered
the same way in its own repository.*

## 17. Prove it

Port the original Bridge's Godot adapter to this design, as a native module. It is a real workload
with a measured baseline, which is why it is the proof. Its probe asserts 191 checks, and two of
them ask another module a question synchronously - `costs.md`, *What we give up*, names both - so
189 is the number that can pass.

It is also the first deployment that needs a clock and a set of phases, and nothing ships either:
`no-builtins.md`, *No builtins*, and `schedule.md`, *Bridge-ASM700 ships no phase names*. Both are
this deployment's own here. Whether either becomes content that deployments share - a module with
a name and a version, a list of phase names - is this stage's question to put, by the method of
`STAGE0.md`, with what the port actually needed in hand.

*Done when 189 of the probe's checks pass, the two that cannot are the two named, and the GDExample
frame is an order of magnitude under 42 us: if it is not, the architecture did not deliver and we
find out here. And when what the port needed of a clock and of phases is written down, with the
answer to whether it is shared.*
