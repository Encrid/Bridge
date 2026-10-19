# src/backends/wasm/wasm_own.c

This file gives a module its own linear memory: a 4 GiB reservation with a 32 MiB guard after it.
wasmtime calls three callbacks to read the memory, grow it, and free it.

## The callbacks

`get_own` reports the committed size, the reserved capacity, and the base.
`grow_own` commits more pages up to the reservation; past it, or on a failed commit, it refuses.
`free_own` releases the whole reservation and guard, then frees the record.
`wasm_make_own` reserves the address space, commits the minimum, and fills the callbacks.
A failed reservation frees the record and gives an error.

## Functions

| Function | Result |
| --- | --- |
| `wasm_make_own` | Reserves a module's own memory and gives wasmtime the callbacks that manage it. |
