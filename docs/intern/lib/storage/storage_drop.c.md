# src/lib/storage/storage_drop.c

This file drops the rows of a component that nobody declares any more, by the policy of its last declarer.
The policies are in `docs/extern/bridge_abi.h.md`, *bridge_on_unload*.
What a strip leaves is in `docs/intern/lib/storage/storage_drop.h.md`, *What unload leaves behind*.

## By layout

`storage_drop` hands each layout to its own function:

| Layout | Function |
| --- | --- |
| A sparse set | `storage_drop_set` |
| Direct-indexed | `storage_drop_cells` |
| A singleton | `storage_drop_single` |
| A table or a toggle | `drop_tables`, which calls `storage_drop_table` for each table that carries the type |

`drop_tables` adds up the rows that `storage_drop_table` leaves in a second table of their set.
If the sum is not 0, `say_apart` writes one warning: `docs/extern/bridge.h.md`, *An unload*.
It finds the name of the component in the declared types of the world, and writes nothing if it does not find it.

## Functions

| Function | Result |
| --- | --- |
| `say_apart` | Writes the one trace of a strip: `docs/extern/bridge.h.md`, *An unload*. Each caller strips a listed type, so it finds its name. |
| `drop_tables` | Drops a component that tables keep, a table at a time. |
| `storage_drop` | Drops the rows of a component that nobody declares any more, by the policy of its last declarer and by its layout. |
