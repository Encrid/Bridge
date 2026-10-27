# src/storage/storage_drop_table.c

This file drops a component from one table that carries it.
Storage destroys the entities, moves them to the table of what is left, or strips the column from the table where it stands.

## One table

`storage_drop_table` makes `left`, the set of the table without the component.
If `left` is empty, `sweep_table` empties the table.
Otherwise storage tells the observers that each entity loses the component, from the last row to the first.
Then `keep_rows` keeps the rows, by the cases of `docs/intern/storage/storage_drop.h.md`, *What unload leaves behind*.

`sweep_table` works from the last row, so that no row moves into the place of a row it saw already.
For each entity, `see_less` writes what the entity carries without the component, the types kept apart included.
`is_swept` then decides from that set: `docs/intern/storage/storage_unload.h.md`, *Functions*.
If `is_swept` gives true, `destroy_entity` destroys the entity, and its observers hear a leave.
Each other entity loses the component, its observers hear it, and it leaves the table for no table.
When the table has no row left, storage gives it back.

Thus a strip whose `left` is empty goes through `sweep_table` as a sweep does, so that a husk with nothing more goes.

## Functions

| Function | Result |
| --- | --- |
| `sweep_table` | Empties a table from its last row. It destroys each entity that `is_swept` gives, and moves each other entity to no table. |
| `tell_rows` | Tells the observers that each entity of a table loses a type, from the last row to the first. |
| `move_rows` | Moves each row of a table to table `to`, which has room for all of them. Then gives back the empty table. |
| `keep_rows` | Keeps the rows of a table without a type, when something that a table keeps is left. Gives how many rows stay apart, in a second table of their set. |
| `storage_drop_table` | Drops a component from one table that carries it, by the policy stated. Gives what `keep_rows` gives, or 0. |
