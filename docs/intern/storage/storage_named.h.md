# src/storage/storage_named.h

This header makes a table the one that its set finds, and puts a table in the list of each system that reads it.

## The hash of tables

The hash has `hash_room` places, a power of two, and a place holds a table id or 0 for none.
A table goes in the first empty place from where `hash_set` points, a place at a time.
When a table leaves, each table after it, up to the next empty place, goes into the hash again.
So a search that stops at an empty place never misses a table.

## The lists of the systems

Each system has a list of the tables that it reads, and `pos` for each table.
A new table goes at the end of the list of each system that reads it.
A table that leaves a list gets the last table of that list in its place.
Each change of a list marks the system `changed`, so storage writes all the spans of its module again.

## Functions

| Function | Result |
| --- | --- |
| `name_table` | Makes table `t` the table that its set finds. |
| `move_home` | Takes out the table at place `k` of the hash, and puts it where a search finds it. A place before it is now empty. |
| `unname_table` | Makes table `t` no longer the table that its set finds. |
| `list_table` | Puts table `t` in the list of each system that reads it. |
| `unlist_table` | Takes table `t` out of the list of each system. |
