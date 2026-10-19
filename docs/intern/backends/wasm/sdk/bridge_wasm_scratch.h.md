# src/backends/wasm/sdk/bridge_wasm_scratch.h

This header gives the scratch arena a module stages a statement or a value in before an entry reads it.
The arena is a region of the private memory, and a module copies a value into it from its own memory.

## The arena

`BRIDGE_STATE` places a struct of a given type in the arena.
`bridge_put` stages bytes in the arena, counts them, and gives the offset; it traps when they do not fit.
`bridge_lend` writes bytes past the used arena but leaves the used length as it was, for a value an entry reads during the call.
`bridge_take_scratch` rounds the used length up to eight and reserves a run of bytes.

## Functions

| Function | Result |
| --- | --- |
| `bridge_put` | Stages bytes in the arena and returns the offset, or traps when they do not fit. |
| `bridge_lend` | Writes bytes past the used arena, for a value read during the call. |
| `bridge_take_scratch` | Reserves a run of bytes in the arena and gives its offset. |
