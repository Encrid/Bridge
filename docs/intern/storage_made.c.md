# src/storage_made.c

This file allocates what storage holds, a part at a time, when Bridge makes a world.
What each budget sizes is in `docs/intern/storage_create.h.md`, *What bounds it*.

## The parts

| Function | What it allocates |
| --- | --- |
| `storage_make_tables` | A place for each entity. For each table id: its record, its free link, its place in `dirty`, its set and its columns. The hash. |
| `storage_make_fold` | For each entity: a set in `work` and in `attached`, its place in `changed`, its flags, and its table to be. The offsets `at`, and 4 more sets. |
| `storage_make_apart` | For each slot of a sparse set: its type, its table and its index `where`. For each direct-indexed slot: its type and its bits `given`. |
| `storage_make_terms` | The types and their records. The records of terms, with their lists and their terms. The lists of systems and of observers. |

There is a table id for each archetype there may be, and one for each sparse set.
The hash has twice the room of the table ids, in a power of two, and at least 16 places.
A part for which the budget asks no room still gets one place, so that `calloc` gets a size above 0.
The lists of types, of sparse sets, of direct-indexed types and of records of terms each have one more place than there may be.
The lists of the systems take two factors in `calloc`, so a product past 64 bits gives no memory, and not a short list.
So a search for a free one always ends.

## Functions

| Function | Result |
| --- | --- |
| `storage_make_tables` | Allocates what storage keeps of entities and of tables. Gives false if a part fails. |
| `storage_make_fold` | Allocates what the fold of a barrier works in. Gives false if a part fails. |
| `storage_make_apart` | Allocates what storage keeps of sparse sets and of direct-indexed components. Gives false if a part fails. |
| `storage_make_terms` | Allocates the types and the terms of the systems. Gives false if a part fails. |
