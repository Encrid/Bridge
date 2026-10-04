# Phase 3: storage

**Not planned yet.** This document exists because an open question waits in the plan of the phase
it blocks - `PHASE0.md` - and phase 2 raised two that are phase 3's.

## Open questions

**What fuel costs.** `limits.md`, *What bounds a run*, bounds every system dispatch by fuel, on the
argument that a count is deterministic and enough. wasmtime's documentation says epoch interruption
runs "up to 2-3x" faster than fuel, since fuel is counted on every instruction. That cost lands on
the path this design exists to protect, and the argument for fuel did not weigh it. To settle: by
measuring a system's loop with fuel on and off, then by the method.

**What the module build must strip.** Binaryen's merge leaves the accessor functions exported from
the merged module - `wasm.md`, *A compiled module reaches them through accessors*. An export
Bridge-ASM700 did not ask for is an entry the scheduler could be handed by name. To settle: how the
build removes them, and whether load refuses a module that still has one.
