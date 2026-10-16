# src/storage_create.h

This header declares the four functions that allocate the parts of storage when Bridge makes a deck.
`src/storage_made.c` defines them, and `src/storage_create.c` calls them.

## What bounds it

A budget gives the size of each thing that storage holds.
Storage allocates all of it when Bridge makes the deck, and nothing after: not at a barrier, and not at a load.

| Budget | What it sizes in storage |
| --- | --- |
| `[budget] entities` | The places of the entities, the fold, and each other thing that storage keeps for an entity. |
| `[budget] tables` | The record of each table, its set, and the hash that finds it. |
| `[budget] columns` | The rows, and `who`, which tells which entity is in which row. |
| `[budget] types` | How wide a set is. With `tables`, the offset of the column of each type in each table. |
| `[budget] systems` | With `terms`, the terms of each system. With `tables`, the list of the tables that each system reads. |
| `[budget] terms` | How long the record of the terms of a system is. |
| `[budget] sparse_sets` | How many sparse sets: two indexes for each, with a cell for each entity, and a table record for each, beside `tables`. |
| `[budget] direct_indexed` | How many direct-indexed components: two sets of bits for each, with a bit for each entity. |

What a module declares takes a place that exists already.
Bridge refuses a `declare` or an `add_system` past its budget, before storage gets it.
The cells of a direct-indexed component and the value of a singleton are the one thing that storage takes at `declare`.
Storage takes them from `[budget] columns`, which exists already.

## Functions

| Function | Result |
| --- | --- |
| `storage_make_tables` | Allocates what storage keeps of entities and of tables. Gives false if there is not the memory for all of it. |
| `storage_make_fold` | Allocates what the fold of a barrier works in. Gives false if there is not the memory for all of it. |
| `storage_make_apart` | Allocates what storage keeps of sparse sets and of direct-indexed components. Gives false if there is not the memory for all of it. |
| `storage_make_terms` | Allocates the types and the terms of the systems there may be. Gives false if there is not the memory for all of it. |
