# src/storage/storage_drop.c

This file drops the rows of a component that nobody declares any more, by the policy of its last declarer.
The policies are in `docs/extern/bridge_abi.h.md`, *bridge_on_unload*.
What a strip leaves is in `docs/intern/storage/storage_drop.h.md`, *What unload leaves behind*.

## By layout

`storage_drop` hands a singleton to `storage_drop_single`, whatever the policy.
A delete of another layout first goes to `storage_delete_carriers`: `docs/intern/storage/storage_husk.c.md`, *A delete at an unload*.
If it leaves carriers, they had no room for a husk, and `say_deleted` writes one warning: `docs/extern/bridge.h.md`, *An unload*.
These carriers then lose only the component: `storage_drop` hands them on with `BRIDGE_STRIP`, in place of `BRIDGE_DELETE`.
`storage_drop` hands each layout but a singleton to its own function, with the policy:

| Layout | Function |
| --- | --- |
| A sparse set | `storage_drop_set` |
| Direct-indexed | `storage_drop_cells` |
| A table or a toggle | `drop_tables`, which calls `storage_drop_table` for each table that carries the type |

Thus the functions of the layouts get `BRIDGE_STRIP` or `BRIDGE_SWEEP`, and never `BRIDGE_DELETE`.
`drop_tables` adds up the rows that `storage_drop_table` leaves in a second table of their set.
If the sum is not 0, `say_apart` writes one warning: `docs/extern/bridge.h.md`, *An unload*.
The two warnings find the name of the component with `get_name`.

## Functions

| Function | Result |
| --- | --- |
| `get_name` | Gives the name of the declared type whose storage type is `type`. Each caller drops a listed type, so it finds the name. |
| `say_apart` | Writes the one trace of a strip: `docs/extern/bridge.h.md`, *An unload*. |
| `say_deleted` | Writes the one trace of a delete that left `count` carriers with no room for a husk: `docs/extern/bridge.h.md`, *An unload*. |
| `drop_tables` | Drops a component that tables keep, a table at a time. |
| `storage_drop` | Drops the rows of a component that nobody declares any more, by the policy of its last declarer and by its layout. |
