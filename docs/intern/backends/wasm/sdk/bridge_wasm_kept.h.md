# src/backends/wasm/sdk/bridge_wasm_kept.h

Some components live outside the table rows: a singleton, a sparse set, or a direct-indexed column.
This header reads those through the kept-storage pointers in the private header.

## Functions

| Function | Result |
| --- | --- |
| `bridge_get_single` | Gives the address of a slot's singleton value. |
| `bridge_get_sparse_index` | Gives the sparse index of a slot: a cell per entity that holds where its value is. |
| `bridge_get_sparse_value` | Gives an entity's value in a sparse index, by the low bits of its id. |
| `bridge_get_cells` | Gives the cells of a direct-indexed component. |
| `bridge_get_cell` | Gives an entity's cell, or zero when its presence bit is clear. |
