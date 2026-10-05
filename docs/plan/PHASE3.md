# Phase 3: storage

**Planned, not built.** This document is the plan; when the phase ends it is rewritten as what was
built and what it found, as `PHASE1.md` and `PHASE2.md` were.

The phase gives a module storage: the shared memory and its private one, the ABI entries that fill
them, a phase that runs systems and the barrier after it. `PLAN.md` phase 3 is the claim.

## What was settled first

Five questions, each by the method of `PHASE0.md`, and each applied to the document it belongs to.

    what fuel costs                   a bound is paid for by the module that states it:
                                      `limits.md`, *What a bound costs*
    what the build strips             the accessor exports, by prefix; load checks nothing:
                                      `wasm.md`, *A compiled module reaches them through accessors*
    releasing an adopted memory       a seventeenth entry, `memory_release`:
                                      `backend.md`, *Memory is adopted, not created*
    the regions nobody sized          two more stated keys, `[budget] columns` and a module's
                                      `spans`: `budgets.md`, *Budgets*
    what running out does             fatal to whoever can be named:
                                      `budgets.md`, *What running out does*

## What it builds

**The backend seam moves three times.** `memory_release`; `module_compile` taking the capabilities a
config demands, with the wasm backend keeping an engine per demand; and a function Bridge-ASM700
provides answering a status, which the wasm backend turns into a trap.

**The host seam grows** by what the claim needs and no more: `phases` and `budget` in
`bridge_config`; `fuel`, `log_level`, `scratch`, `log_ring`, `spans` and the two grant lists in a
module's config; and `bridge_phase`. Order, unload, reload and the pulls stay where `PLAN.md` put
them.

**Storage.** One shared block: a reserved first page, the entity index, the persistent arena, the
columns. flecs is created over it, its allocator carving the column region in power-of-two classes
whose free lists live outside the block. One private block per module: header, scratch arena, log
ring, and a span region divided again at every barrier into query slots, `column_base` and spans.

**The ABI.** `declare`, `system`, `spawn`, `destroy`, `attach`, `detach` and `alloc`. `observe` is
defined so a module that imports it links, and refuses until phase 5.

**A phase and its barrier, serial.** Systems of the phase in registration order, fuel set before
each. Then: flecs' queue flushed, the mirror synced from the hook and the events, `column_base`
rebuilt if a column moved, spans published with the row-count check, rings drained to the sink,
scratch arenas reset.

**A module's side.** `bridge-wasm.h` and `bridge-wasm.wat`: the imports, the `ECS_STRUCT` shim, the
layouts read with plain loads, and the accessors. The build compiles a module with zig, merges the
accessors with Binaryen, inlines, and strips.

## Steps

1. Documents: the five decisions. Committed before any code.
2. The seam: `bridge-backend.h`, `wasm.c`, `tests/adopt.c`. Phase 2's tests pass again.
3. The module build: the header, the accessors, CMake. A compiled module whose system has plain
   loads in its loop and no call - checked by reading the built module.
4. `bridge-abi.h`: the layouts. The shared block, the allocator, the world.
5. Load: private memory, the entries, the checks at `declare` and `system`.
6. `bridge_phase` and the barrier.
7. The tests below, one case per clause of the claim.
8. The accessor measurement, written into `validation.md`.
9. Close: this document rewritten, `PLAN.md` marked, scan.

## The tests

Every refusal is checked by the record it emits, and every case ends by printing "alive".

    share        two modules, one column: the writer's change is the reader's sum
    spans        each module's query sees its own tables, and not the other's
    nocall       the built system has no call in its loop
    mirror       ids followed after a batch of destroys: survivors right, the dead refused
    fingerprint  a second declarer with another struct body, refused naming both
    grant        a declare in another namespace with no grant, refused by name
    wildcard     a wildcard term, refused by name
    fuel         a system that never returns, stopped, and the next phase still runs
    ceiling      a spawn past the entity ceiling abandons that invocation
    log          a line reaches the sink with its module and system
    ring         an overflowing ring reports how many it dropped
    nospans      a module whose spans do not fit is stopped, and the other runs on
    nocolumns    columns running out stops the bridge, and destroy still works
    bench        the accessor's test, against a build without it

## What would make us stop

Binaryen not removing the call from the loop: then the accessor route fails its own claim, and
`wasm.md` names what comes back. The mirror disagreeing with flecs after the workload of `mirror`:
then one hook is not enough for what Bridge-ASM700 does, and `flecs.md` is wrong about it.

## Not in this phase

Named so nobody looks for them here: the conflict graph and `[order]`, observers, threads, `on_error`
past init and the failure budget, unload and reload, `[.config]`, the checksum pass and the
profile. A nested struct in a component is not resolved either: a descriptor may use the scalar
types, enums' underlying types and inline arrays.
