# src/lib/storage/storage_make.h

This header makes a table for a set that has none, and gives a table back when its last row left.

## A table made

`make_table` gives the table of a set: the one that `find_table` finds, or a new one.
A set that holds the type of a sparse set is the table of that set.
Storage has room for the table of each sparse set beside `[budget] tables`, so it never refuses one of those.

`make_table` gives 0 for a table of an archetype past `[budget] tables`.
A table id is always free then, since live tables stay below `table_room`: `[budget] tables` and one table for each sparse set.
A new table is live, with no row and no column yet.
Storage puts it in `dirty`, in the hash, and in the lists of the systems: `docs/intern/lib/storage/storage_named.h.md`.

## A table given back

`free_table` gives the block of the table back to the column region, and removes the table from the hash and from each list.
It makes the columns of the table 0, marks it touched, and puts its id on the free list.
`close_table` makes the record no table, and keeps what a barrier keeps of it: `incoming`, `sized` and `touched`.
An id given back at this barrier is in `dirty` already, so `open_table` keeps `touched` when it takes the id again.

## Functions

| Function | Result |
| --- | --- |
| `close_table` | Makes a table no table: not live, with no row, no room, no block and no sparse set. |
| `take_table_id` | Gives a table id that nobody has: one given back, or the next one never used. |
| `open_table` | Makes table `t` the live table of a set, with no row and no column. Writes its set, and puts it in the hash and in the lists. |
| `make_table` | Gives the table of a set, made if there is none. Gives 0 if there is no room for it. |
| `free_table` | Gives back a table whose last row left. |
