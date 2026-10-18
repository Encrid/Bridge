# src/lib/storage/storage_drop_table.c

This file drops a component from one table that carries it.
Storage destroys the entities, moves them to the table of what is left, or strips the column from the table where it stands.

## One table

`storage_drop_table` makes `left`, the set of the table without the component.
`is_swept` tells whether the policy destroys an entity with the set `left`: `docs/intern/lib/storage/storage_unload.h.md`, *Functions*.
If it does, `sweep_table` empties the table.
Otherwise storage tells the observers that each entity loses the component, from the last row to the first.
Then `keep_rows` keeps the rows, by the cases of `docs/intern/lib/storage/storage_drop.h.md`, *What unload leaves behind*.

`sweep_table` works from the last row, so that no row moves into the place of a row it saw already.
Under `BRIDGE_DELETE`, it destroys each entity.
Under `BRIDGE_SWEEP`, an entity that has something that no table keeps still carries something.
So storage does not destroy it: the entity loses the component, its observers hear it, and it leaves the table for no table.
When the table has no row left, storage gives it back.

## Functions

| Function | Result |
| --- | --- |
| `sweep_table` | Empties a table whose entities the policy destroys, from the last row. |
| `tell_rows` | Tells the observers that each entity of a table loses a type, from the last row to the first. |
| `move_rows` | Moves each row of a table to table `to`, which has room for all of them, or to no table. Then gives back the empty table. |
| `keep_rows` | Keeps the rows of a table without a type. Gives how many rows stay apart, in a second table of their set. |
| `storage_drop_table` | Drops a component from one table that carries it, by the policy stated. Gives what `keep_rows` gives, or 0. |
