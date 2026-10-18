# src/lib/storage/storage_create.c

This file makes the storage of a world, all of it or none, and frees it.
What each budget sizes is in `docs/intern/lib/storage/storage_create.h.md`, *What bounds it*.

## The sizes

`set_sizes` takes from the budget how much of each thing storage may hold:

| Field | Size |
| --- | --- |
| `table_room` | `[budget] tables` plus `[budget] sparse_sets`. |
| `set_room` | `[budget] sparse_sets`. |
| `direct_room` | `[budget] direct_indexed`. |
| `bit_words` | A bit for each of `[budget] entities`, in 64-bit words. |
| `words`, `type_room` | A bit for each of `[budget] types`, in 64-bit words, and those words counted in bits. |
| `terms_room` | `[budget] systems`. |

Table 0 is no table and index 0 is no entity, so `table_high` and `high` start at 1.
The budget check bounds these counts, so no size here wraps: `docs/intern/lib/block/block_budget.c.md`, *The checks*.

## Out of memory

`storage_create` asks for each part before it keeps any.
If one part fails, it writes a record that says what lacked, frees what it made, and gives `nullptr`.
`who` has a cell for each 8 bytes of the column region.
The ids of a table are at the start of its block, so their twin is at the same place in `who`.
`os_reserve` reserves and commits all of `who`, and the operating system gives memory only to what storage touches.
If the operating system does not give `who`, `storage_create` writes that `[budget]` cannot be satisfied, and gives `nullptr`.

## Functions

| Function | Result |
| --- | --- |
| `set_sizes` | Takes from the budget how much of each thing storage may hold. |
| `make_all` | Allocates the parts of storage with the four functions of `src/lib/storage/storage_create.h`. Gives false at the first that fails. |
| `storage_create` | Makes the storage of a world, all of it or none. Gives `nullptr`, with a record, if it cannot. |
| `free_fold` | Frees what the fold, the sparse sets and the direct-indexed components hold. |
| `storage_destroy` | Gives `who` back to the operating system, and frees each part of storage and its record. |
