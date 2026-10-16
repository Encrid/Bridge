# src/storage_declare.c

This file declares a type: it gives the type its number, its slot if no table keeps it, and its block if it needs one.
The storage layouts are in `docs/intern/storage.h.md`, *Storage layouts*.

## A declare

The type takes the lowest free id.
A type of a sparse set takes the first free slot of the sparse sets, and a direct-indexed type the first free slot of those.
The declarer asked first how many are left, so a slot is free.

A direct-indexed type and a singleton take a block from the column region, made zero.
That block is the one thing that a `declare` takes from `[budget] columns`:

| Layout | The block |
| --- | --- |
| Direct-indexed | 16 bytes, then a cell for each of `[budget] entities`. The first 4 bytes hold the offset of its bits for modules. |
| A singleton | Its one value. A tag counts as 1 byte, so the block is never empty. |

The length of each block is rounded up to a multiple of 16.
A direct-indexed type makes its bits and their copy for modules 0.
If the column region has no room for the block, or the block is longer than `INT32_MAX`, `storage_declare` gives `nullptr`.

## Functions

| Function | Result |
| --- | --- |
| `find_slot` | Gives a free slot for a type of a sparse set or a direct-indexed type, and 0 for any other. |
| `take_zeroed` | Takes `want` bytes of the column region, made zero. Gives `nullptr` if there is no room. |
| `take_cells` | Takes the block that a direct-indexed type or a singleton needs. Gives false if `[budget] columns` has no room for it. |
| `file_type` | Counts a declared type where its storage layout is counted: in the sparse sets, in the direct-indexed types, or in `in_table`. It also counts the types kept apart and the tags. |
| `storage_declare` | Declares a type of `size` bytes, kept in the layout `kept`. Gives the record of the type, or `nullptr` if its block has no room. |
