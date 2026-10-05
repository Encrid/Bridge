# Phase 3: storage

Done. The `linux` preset was first run after phase 5, and passed as written.

The phase gave a module storage: the shared memory and its private one, the ABI entries that fill
them, a phase that runs systems and the barrier after it. `PLAN.md` phase 3 is the claim.

What each piece is and why is in the documents it belongs to: `memory.md`, `budgets.md`, `abi.md`,
`query.md`, `reference.md`, `schema.md`, `wasm.md`, `flecs.md`. This document is what the phase
built and what it found on the way.

## What was settled first

Five questions, each by the method of `PHASE0.md`, and each applied to the document it belongs to.
Three were waiting here from phase 2; two turned up while planning.

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

## What it produced

    include/bridge-backend.h      memory_release, module_compile's demands, a status from a
                                  function Bridge-ASM700 provides
    backends/wasm/wasm.c          an engine per demand; a refused entry becomes a trap
    include/bridge.h              phases, [budget], a module's fuel, log level, scratch, ring,
                                  spans and grants; bridge_phase
    include/bridge-abi.h          the layouts: header, query slot, span, index record, log
                                  record, term
    include/bridge-wasm.h         what a wasm module in C compiles against
    include/bridge-wasm.wat       the accessors
    src/store.c                   the shared block, the column allocator, the mirror, publishing,
                                  the ring drain, the barrier
    src/abi.c                     the eight entries
    src/bridge.c                  create, load, phase
    tests/modules/*.c             one module per case, compiled by zig and merged by Binaryen
    tests/phase3.c                a host that links the library and nothing else
    tools/nocall.py               reads a built module: its system loads, stores, and calls nothing

zig and Binaryen joined the tools the test build needs. Binaryen is fetched at its pin, with its
hash, when the machine has none.

## The tests

Seventeen new ones, beside the twenty of phases 1 and 2. All thirty-seven pass in a release build
and in a debug one, where flecs' own assertions are on.

    share        two modules, one column: the reader's sum moves by what the writer added
    spans        the writer's query matches two tables and 1,000 rows, the reader's one and 500
    nocall       the built system: ten loads and stores, no call
    log          a line reaches the sink with its module and its system
    mirror       100 ids followed across two batches of destroys: 100 right, 0 wrong, each time
    fingerprint  another struct body under one name: refused, naming both fingerprints
    grant        a declare in another namespace with no grant: refused by name
    wildcard     a wildcard term: refused by name
    fuel         a system that never returns: "all fuel consumed", twice, and the next system
                 of the same module runs both times
    ceiling      a spawn past [budget] entities abandons that invocation, by name
    ring         100 lines into a ring of 16: 16 arrive, "84 log lines were dropped"
    nospans      a module whose spans do not fit is stopped, and the other runs on
    nocolumns    columns running out stops the bridge; a phase is then refused; destroy works
    nophase      a phase the deployment did not declare: refused by name
    nophases     a deployment with no phase: create refuses
    nobudget     a budget left unset: create refuses, naming the key
    bench        the accessor's test, against a build without it

## What the phase found

**Nothing needed generating.** The plan had a schema generator emitting typed accessors. An
accessor needs a field's type, offset and struct size, and a module's own compiler has all three:
`_Generic`, `offsetof`, `sizeof`. So what a module compiles against is a header and a file of wasm
text, and no tool reads a schema - `schema.md`, *What a module compiles against*.

**The accessor's test is not free, and under fuel it costs more than itself.** `validation.md`,
*Always on*, had reasoned that a compiler would hoist it. Neither Binaryen nor wasmtime does. It
stays, with the measurement there beside it and the condition that reopens it.

**The merge goes both ways.** The accessors copy bytes between a module's own memory and the other
two, so the accessor module imports the compiled module's memory while the compiled module imports
the accessors. Binaryen resolves both in one merge.

**flecs' own tables take columns before any module declares anything**, so `[budget] columns` has
a floor that is not ours - `budgets.md`, *Bridge-ASM700 defaults none of them*, for the figure and
what create does about it.

**Table ids did not need flecs' table events.** A table gets its id the first time the mirror
places an entity in it or a query matches it, which covers every table a module can reach.
`EcsOnTableDelete` waits for unload, the first thing that deletes one - `flecs.md`, *flecs will not
tell you which*.

**One hook was enough, again.** 100 ids, two batches of destroys that each move the last row into
a hole: every survivor found at its row, every dead id refused by its generation, and the
row-count check of `validation.md`, *Always on*, silent throughout.

**A module's header has to be written before `init`, not at the first barrier.** `init` is the
first thing to bump the scratch arena, and reads where it is from the header. Found by every module
trapping in `init`.

## What it left

Nothing, any more. The phase closed with a failed load leaving what its `init` had done, an
allocator in power-of-two classes, one bridge to a process, and `observe` refusing - `PHASE5.md`,
*Closed after the phase*, for each.

## Open questions

None for the next phase. The native backend will meet one this phase named and did not build: an
entry that refuses cannot unwind native code, so there the call returns and the invocation is failed
afterwards - `signatures.md`, *Imports are declared, exports are checked*.

## What would make us stop

Neither happened. Binaryen removes the call from the loop, which `tools/nocall.py` checks on every
build. The mirror agrees with flecs after every batch.
