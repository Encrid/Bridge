# src/lib/storage/storage_reach.h

This header gives where storage keeps what it knows of a table, a sparse set and a direct-indexed component.
It also gives what a module reads of each.
The two indexes of a sparse set are in `docs/intern/lib/storage/storage_inside.h.md`, *A sparse set*.
The bits of a direct-indexed component are in `docs/intern/lib/storage/storage_inside.h.md`, *Direct-indexed*.

## Functions

| Function | Result |
| --- | --- |
| `get_columns` | Gives where the column of each type is in table `t`, by type id. That is an offset from the shared base, or 0 for none. |
| `get_where` | Gives, for each entity, its row in the sparse set of `slot` plus one, or 0. Storage keeps this index. |
| `get_cells` | Gives what a module reads of the sparse set of `slot`: for each entity, the offset of its value from the shared base, or 0. |
| `get_given` | Gives which entities have a value of the direct-indexed component of `slot`: a bit for each entity. Storage keeps these bits. |
| `get_shown` | Gives the copy of the same bits that a module reads, in the shared memory. The 16 bytes before the cells give its offset. |
