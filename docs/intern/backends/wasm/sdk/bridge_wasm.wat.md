# src/backends/wasm/sdk/bridge_wasm.wat

This file holds the accessors a compiled module reaches Bridge's two other memories through.
A wasm instruction names the memory it loads, and no C compiler emits a load from a memory that is not the module's own.
So the accessors are written by hand in wasm text, where each load and store names its memory.

## The three memories

The module imports three memories: the shared ECS data, its own private data, and its own linear memory.
`acc_row` gives a row's address, and the checked form traps when the row is past the span's count.
The load and store accessors read and write the shared memory by width, and the `acc_own` accessors read and write the private memory.
Three copy accessors move bytes among the module's own memory, the shared, and the private.

## The build

The module build merges this file into the compiled module as `acc`, inlines every accessor into its callers, and strips the exports.
`docs/intern/backends/wasm/sdk/bridge_wasm_acc.h.md` declares the accessors a module calls.
