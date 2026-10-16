# src/storage_drop.c

This file drops the rows of a component that nobody declares any more, by the policy of its last declarer.
The policies are in `docs/intern/storage_drop.h.md`, *What unload leaves behind*.

## By layout

`storage_drop` hands each layout to its own function:

| Layout | Function |
| --- | --- |
| A sparse set | `storage_drop_set` |
| Direct-indexed | `storage_drop_cells` |
| A singleton | `storage_drop_single` |
| A table or a toggle | `drop_tables`, which calls `storage_drop_table` for each table that carries the type |

`drop_tables` adds up the rows that stay in a second table of their archetype.
If there are some, `say_apart` writes one warning, which names the component, the rows and `[budget] columns`.
It finds the name of the component in the declared types of the world, and writes nothing if it does not find it.

## Functions

| Function | Result |
| --- | --- |
| `say_apart` | Writes the one trace of a strip: the rows that stay in a second table of their archetype. |
| `drop_tables` | Drops a component that tables keep, a table at a time. |
| `storage_drop` | Drops the rows of a component that nobody declares any more, by its layout. |
