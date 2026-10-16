# src/storage_reserve.h

This header finds a table for each changed entity before anything moves.
It is the first part of the second step of a barrier, and of a move: `docs/intern/storage_room.h.md`, *Functions*.

## A table for each entity

The table of an entity holds what the entity will carry that tables keep, and no more.
While a type is kept apart, `get_wanted` masks the set of the fold with `in_table` into `masked`.
An entity whose table already has that set stays where it is.
So an attach of a component that an entity has is a value and no move, in whichever table of that set the entity is.

Otherwise storage finds or makes the table of the set, and counts the entity among the rows that table gains.
An entity that will carry nothing that tables keep goes to no table.
If storage cannot make the table, it leaves the entity as it was.

An entity that joins a sparse set counts as a row that the table of the set gains.
Storage makes that table if no entity is in the set.
There is room for a table for each sparse set, so storage never refuses that one.

## Functions

| Function | Result |
| --- | --- |
| `get_wanted` | Gives the set of the table that the entity at `index` is to be in. |
| `pick_table` | Finds the table that the entity at `index` is to be in, made if there is none, and counts the entity among its new rows. Gives false, with the entity left as it was, if there is no room for the table. |
| `reserve_join` | Counts the entity at `index` among the rows that the sparse set of `slot` gains. Makes the table of the set if it has none. |
| `count_joins` | Counts a row in each sparse set that the entity at `index` is to join. |
| `reserve_one` | Finds where the entity at `index` is to be while it is still where it was: its table, and each sparse set it joins. Does nothing for a destroyed entity. |
