# src/backends/native/native_phase.c

This file gives the five entries a module reaches while it runs in a phase.
Each entry gathers its arguments as an array and reaches `native_enter`.
`docs/intern/backends/native/native_inside.h.md`, *The invariant*, names that step.

## Functions

| Function | Result |
| --- | --- |
| `native_spawn` | Reaches `spawn` and gives the new entity id. |
| `native_destroy` | Reaches `destroy` with an entity id. |
| `native_attach` | Reaches `attach` with an id, a slot and a scratch offset. |
| `native_detach` | Reaches `detach` with an id and a slot. |
| `native_alloc` | Reaches `alloc` with a length and gives an offset. |
