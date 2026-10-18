# src/lib/storage/storage_value.h

This header gives where an entity has its value of a type, in each storage layout.
It also has an entity join and leave a sparse set.

## Where a value is

Each place is an offset from the shared base, and 0 says that the entity has no value:

| Layout | Where the value of the entity at `index` is |
| --- | --- |
| A table or a toggle | Its row in the column of its table. 0 if it is in no table, or if its table has no such column. |
| A sparse set | Its row in the table of the set, through `where`. 0 if it is not in the set. |
| A singleton | The block of the singleton, while it has a value. |
| Direct-indexed | Cell `index`, after the 16 bytes before the cells, while the entity has a value. |

An entity that joins a sparse set gets a row at the end of the table of the set.
Its value is zero until an attach writes one.
When an entity leaves a sparse set, the last row of the set takes its place.

## Functions

| Function | Result |
| --- | --- |
| `get_sparse_value` | Gives where the entity at `index` has its value in a sparse set, or 0 if it is not in it. |
| `get_table_value` | Gives where the entity at `index` has its value in its table, or 0. |
| `get_value` | Gives where the entity at `index` has its value of a type, in each layout, or 0 if it has none. |
| `join_set` | Has the entity at `index` join the sparse set of `slot`, whose table has room for it, with a value of zero. |
| `leave_set` | Has the entity at `index` leave the sparse set of `slot`. Its two cells become 0, and the last row of the set takes its place. |
