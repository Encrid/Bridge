# src/storage/storage_made.c

This file allocates what storage holds, a part at a time, when Bridge makes a world.
What each budget sizes is in `docs/intern/storage/storage_create.h.md`, *What bounds it*.
`type_room` is never 0: `[budget] types` is at least 1, and storage rounds it up to 64.

## The parts

| Function | What it allocates |
| --- | --- |
| `storage_make_tables` | A place for each entity. For each table id: its record, its free link, its place in `dirty`, its set, its columns and a hash for each column. The lookup hash. |
| `storage_make_fold` | For each entity: a set in `work` and in `attached`, its place in `changed`, its flags, its table to be, and its bit in `husks`. The offsets `at`, and 5 more sets. |
| `storage_make_apart` | For each slot of a sparse set: its type, its table and its index `where`. For each direct-indexed slot: its type and its bits `given`. |
| `storage_make_terms` | The types and their records. The records of terms, with their lists and their terms. The lists of systems and of observers. |

There is a table id for each archetype there may be, and one for each sparse set.
A part for which the budget asks no room still gets one place, so that `calloc` gets a size above 0.
The lists of types, of sparse sets, of direct-indexed types and of records of terms each have one more place than there may be.
The lists of the systems take two factors in `calloc`, so a product past 64 bits gives no memory, and not a short list.
So a search for a free one always ends.

## The hash of tables

The hash has a power of two of places: the least one where the `tables` table ids fill at most `table_hash_fill` percent of the places.
`table_hash_fill` is a key of the deployment, from 1 to 99, and the product is in 64 bits:

```pseudo
hash_room <- 1
WHILE hash_room * table_hash_fill < tables * 100
   hash_room <- hash_room * 2
ENDWHILE
```

A fill below 100 leaves an empty place, where a search of the hash stops: `docs/intern/storage/storage_named.h.md`, *The hash of tables*.

## Functions

| Function | Result |
| --- | --- |
| `storage_make_tables` | Allocates what storage keeps of entities and of tables. Gives false if a part fails. |
| `storage_make_fold` | Allocates what the fold of a barrier works in. Gives false if a part fails. |
| `storage_make_apart` | Allocates what storage keeps of sparse sets and of direct-indexed components. Gives false if a part fails. |
| `storage_make_terms` | Allocates the types and the terms of the systems. Gives false if a part fails. |
