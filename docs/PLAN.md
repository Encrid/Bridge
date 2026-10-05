# Plan

The build order, and nothing else. What the design is and why is `DESIGN.md`; what the documents are
is `README.md`.

**Every phase ends in a claim that can be false.** A phase is not done because its code exists. It
is done when the stated thing is observably true. A phase whose done-when cannot fail will be
declared finished while broken.

**No done-when names something a later phase builds.** A phase is tested with what exists when it
ends, so each one carries the part of the host seam, the sink and the scheduler its own claim needs.

## 0. Documents, and the spikes they need

Done. The design documents `README.md` lists were written, the register of open items was emptied
and deleted, and the documents left draft.

Two questions could not be settled by argument, so a throwaway spike settled them inside this phase:
whether flecs can be confined to our block, and whether its events are enough to keep the entity
mirror in step. Both answered with numbers, and the second answered *no* - `flecs.md`, *It can be
confined*, and *flecs will not tell you which*.

*Done when the register is empty and every design document is out of draft.* Met.

## 1. Vendor and patch flecs

Done, on Windows. flecs v4.1.6, vendored pristine as the amalgamation and patched by a script: the
allocator argument at the twenty-three calls that allocate a column or an entity array, and one
row-move hook - `flecs.md`, *It can be confined*, *flecs will not tell you which*, and *How it is
carried* for the shape. The build is CMake with Ninja presets, and it starts in this phase.
`PHASE1.md` is what it built and found.

**First, because everything else needs it.** Columns cannot be in the shared memory without the allocator,
and the mirror cannot be correct without the hook.

*Done when ctest's confinement test fails if one pointer escapes the block, its row-move test fails
if one moved record goes unreported, and the patch script declines a flecs version it was not
written for.* Met; the `linux` preset is unrun.

## 2. One module runs

Done, on Windows. `PHASE2.md` is what it built and found.

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
opened and a null sink are each refused with a record naming the cause.* Met; the `linux` preset is
unrun, and the shared memory is read by a test of the backend alone, since storage is phase 3's.

## 3. Storage

Done, on Windows. `PHASE3.md` is what it built and found.

The shared memory's three regions and each module's private memory: the header, the spans,
`column_base`, the scratch arena and the log ring - `memory.md`, *Where the seven regions live*.
`[budget]`, with exhaustion fatal where it happens - `budgets.md`, *What running out does*.

`declare`, with the fingerprint, type-set and grant checks. `system`, with the query refusals -
`query.md`, *What a query may not ask for*. The structural entries `spawn`, `destroy`, `attach` and
`detach`, and `alloc`. Fuel, re-set before every dispatch. `bridge_phase` and the barrier, serial
for now: queued changes applied, spans republished, rings drained, scratch reset. The entity index
mirror, driven off the patched hook and flecs' events, and `get` over it - `reference.md`,
*Reference following*. The row-count check - `validation.md`, *Always on*. What a module compiles
against, and the module build that merges it: the `ECS_STRUCT` shim and accessors carrying the
`row < count` test - `schema.md`, *What a module compiles against*.

Access mode and queries are part of declaration from the start, not retrofitted.

*Done when two modules share a component array with no crossing per access, neither can reach the
other's spans, a mirror lookup survives a batch of swap-removes, and the accessor's test is measured
against a build without it* - `validation.md`, *Always on*, states that cost as reasoning, and this
is where it becomes a measurement. *And when a fingerprint mismatch, an ungranted declare and a
wildcard term are each refused by name; a system that never returns is stopped by its fuel; a
`spawn` past the entity ceiling fails where it happens; a log line reaches the sink; and an
overflowing ring reports how many it dropped.* Met; the `linux` preset is unrun.

## 4. The native backend

Done, on Windows. `PHASE4.md` is what it built and found.

The second backend: a shared library that opens a module's library and calls it directly -
`native.md`. `bridge-native.h`, `bridge_bind`, and exports found by name as `bridge_<name>`. Nothing
changes above the backend seam: a native module's systems are scheduled like any other, it reads
the same spans through `private` and `shared`, and its log ring is reached through a pointer.

*Done when a native module iterates a component array through a raw pointer; a native system and a
wasm system run in the same phase with the scheduler unable to tell; and a module that
sets `trusted = false` or any limit is refused on the native backend, by the capability it
lacks, before its library is opened.* Met; the `linux` preset is unrun.

## 5. Scheduling

The conflict graph, and the refusal when two conflicting systems in one phase are left unordered.
`[order]`. `observe`, with observers at the barrier, one hop
per barrier, and the reserved names refused. Threads running non-overlapping writers, which the
declared sets give us, and `pinned` - `pinned.md`, *Threads*.

*Done when a deployment orders two conflicting systems with `[order]` alone, an ambiguous pair is
named instead of guessed, a component a native module sets runs a wasm module's observer at that
same barrier, a three-deep observer chain settles in three barriers rather than hanging, and two
modules provably run in parallel. And when every system of a pinned module reports one thread id.*

## 6. Failure and lifecycle

`on_error`, `max_system_failures` and the stop, and the two escalation guards - `failure.md`, *When
something fails*. `bridge_unload` and `bridge_reload` with their rules, `hot_reload`, and the reload
and shutdown timeouts - `lifetime.md`, *Lifetime*. `[.config]` seeding a singleton component -
`config.md`, *A module's own settings are a component*. `bridge_modules`, because a stop is the
state it exists to report - `introspection.md`, *Introspection*.

*Done when a skipped `init` leaves the other modules running; a refused load leaves the schedule and
storage as they were; a module past its failure budget
shows as STOPPED; a reload keeps a row's bytes; a changed layout is refused naming both
fingerprints; an unload drops an exclusive component and keeps a shared one; and a `[.config]` key
the module never declared is an error naming it.*

## 7. A host, from outside

A standalone host, built in its own repository against the library and nothing else, which is what
tests the host seam: if it needs no part of Bridge-ASM700's inside, the seam is real. Whatever it
reads - files, layers, a manifest - it resolves into the structs of `host-api.md`, *The data*.
`bridge_manifest_parse` ships beside the library, not inside it, for a host that wants it -
`native.md`, *Identity*.

*Done when that host drives a real deployment for one pass and for many; it reloads a changed module
and reports one that failed to load; a key it left unset is an error naming the key; and
`bridge_manifest_parse` reads an identity out of both a wasm binary and a native library.*

## 8. Diagnostics

`[checks] validate` and `[checks] profile` - the barrier's checksum pass and the per-system
counters - and `bridge_profile`, a pull. `validation.md`, *The checksum pass, when asked*, and
`profile.md`, *The profile*.

*Done when a deliberate stray write is reported with the module that made it, a system made slow on
purpose is the one the profile names, and an observer cycle shows as call counts that never fall to
zero.*

## 9. Prove it

Port the original Bridge's Godot adapter to this design, as a native module. It is a real workload
with a measured baseline, which is why it is the proof. Its probe asserts 191 checks, and two of
them ask another module a question synchronously - `costs.md`, *What we give up*, names both - so
189 is the number that can pass.

*Done when 189 of the probe's checks pass, the two that cannot are the two named, and the GDExample
frame is an order of magnitude under 42 us. If it is not, the architecture did not deliver and we
find out here.*

## Not in this plan

Each is refused or deferred in the document named, with the condition that would bring it in.

    the journal, record and replay   `schedule.md`, *What a pass depends on*
    a default phase set              `schedule.md`, *Bridge-ASM700 ships no phase names*
    bridge:clock@1 and its kind      `no-builtins.md`, *No builtins*
    per-component page mapping       `costs.md`, *What we give up*
    a phase exclusivity flag         `schedule.md`, *A phase is a name and nothing else*
    memory high-water per module     `profile.md`, *The profile*
    an instance-grouping entry       `backend.md`, *What the backend does not do*
    arena mark and reset             `strings.md`, *It is never reclaimed*
    layout migration                 `lifetime.md`, *When the layout changed*
    bindings for a non-C module      `wasm.md`, *The published layouts*
    upstreaming the allocator hook   `flecs.md`, *How it is carried*
