# src/backends/wasm/sdk/bridge_wasm_acc.h

This header declares the host calls a module imports and the accessors that reach its two other memories.
`docs/intern/backends/wasm/sdk/bridge_wasm.wat.md` gives the accessors; this header names them for C.

## The imports and the macros

`BRIDGE_IMPORT`, `BRIDGE_ACC` and `BRIDGE_EXPORT` mark a function as a host call, an accessor, or a module export.
The host calls are the load-time and phase entries: declare, add_system, observe, spawn, destroy, attach, detach and alloc.
`BRIDGE_HEADER` reads a field of the private header by its offset.
A module states `BRIDGE_ROWS_CHECKED` or `BRIDGE_ROWS_UNCHECKED`, which picks the checked or unchecked `acc_row`.

## Functions

| Function | Result |
| --- | --- |
| `acc_row` | Gives a row's address in the shared memory, checked or unchecked by the module's choice. |
| `acc_ld_*` and `acc_st_*` | Load and store the shared memory by width and type. |
| `acc_own_32`, `acc_own_64` and `acc_own_st` | Load and store the private memory. |
| `acc_to_private`, `acc_to_shared` and `acc_from_shared` | Copy bytes between the module's own memory and the other two. |
