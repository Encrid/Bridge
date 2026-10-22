# include/backends/wasm/bridge_wasm_acc.h

This header declares the host calls a module imports and the accessors that reach its two other memories.
`docs/intern/backends/wasm/sdk/bridge_wasm.wat.md` gives the accessors; this header names them for C.

## The imports and the macros

`BRIDGE_IMPORT`, `BRIDGE_ACC` and `BRIDGE_EXPORT` mark a function as a host call, an accessor, or a module export.
The host calls are the load-time and phase entries: declare, add_system, observe, spawn, destroy, attach, detach and alloc.
`BRIDGE_HEADER` reads a field of the private header by its offset.

## Functions

| Function | Result |
| --- | --- |
| `acc_ld_*` and `acc_st_*` | Load and store a cell of the shared memory, by width and sign. |
| `acc_entity` | Packs an id into its table and row, or returns all ones for a dead id. |
| `acc_who` | Gives the id that owns a table and a row. |
| `acc_has` | Gives 1 when a slot kept outside a table is present for an id. |
| `acc_toggle` and `acc_toggle_set` | Read and set a row's bit in a toggle. |
| `acc_own_32`, `acc_own_64` and `acc_own_st` | Load and store the private memory. |
| `acc_to_private`, `acc_to_shared` and `acc_from_shared` | Copy bytes between the module's own memory and the other two. |
