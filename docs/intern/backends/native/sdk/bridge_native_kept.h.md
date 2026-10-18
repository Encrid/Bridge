# src/backends/native/sdk/bridge_native_kept.h

This header reads the components a table does not keep, through real pointers from the two bases.
It covers a singleton, a direct-indexed component, a sparse set, the bits of a toggle, and an observer's list.

## Functions

| Function | Result |
| --- | --- |
| `bridge_native_get_single` | Gives the singleton of a slot, or null while it has no value. |
| `bridge_native_get_cells` | Gives the cells of a direct-indexed component, one per entity by the low bits of its id. |
| `bridge_native_get_sparse_index` | Gives the index of a sparse set: a cell per entity, holding where its value is, or zero. |
| `bridge_native_get_bits` | Gives the words of a toggled column, where each row is one bit. |
| `bridge_native_get_list` | Gives an observer's list of trigger records. |
