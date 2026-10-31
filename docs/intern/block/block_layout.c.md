# src/block/block_layout.c

This file finds the `block_layout_t` of the shared block: `docs/intern/block/block.h.md`, *Types*.

## The parts

| Part | Begins at | Holds |
| --- | --- | --- |
| None | 0 | Eight bytes with nothing in them, so that offset 0 is none and the entity index is aligned. |
| `index` | 8 | The entity index: a `bridge_entity_t` of twelve bytes for each of `[budget] entities`. |
| `sparse` | A multiple of 8 | For each sparse set, its index: a `u32` for each entity. |
| `direct` | A multiple of 8 | For each direct-indexed component, its bits: a bit for each entity, in words of 64 bits. |
| `directory` | A multiple of 8 | The table directory: a `u32` row count for each table id, then a `u32` block for each table id. |
| `arena` | A multiple of 8 | The persistent arena: `[budget] persistent` bytes. |
| `columns` | A multiple of 16 | The column region: `[budget] columns` bytes. |

A count of `BRIDGE_NONE` gives no sparse set or no direct-indexed component, and its part is empty.
The table directory has `[budget] tables` plus the sparse sets plus 1 entries in each array: one for each table id.
The layout rounds the length of the block up to a whole number of the largest granule that a backend states: `docs/extern/bridge_backend.h.md`, *Memory*.

The layout first refuses `entities`, `persistent` or `columns` above `UINT32_MAX`.
Then each amount of the budget is below 2^32, so no sum can be past 64 bits.

## The limits

Each offset into the shared block is 32 bits, so the layout refuses these things:

- `entities`, `persistent` or `columns` above `UINT32_MAX`, by its key, before any sum.
- Sparse sets whose indexes, at four bytes for each entity, are past what an offset reaches.
- Direct-indexed components whose bits are past what an offset reaches.
- A block longer than 2^32 bytes.

Each refusal has a record that says the budget cannot be satisfied, and why.

## The address space

The block is reserved in the largest reservation that any backend stated, with the largest guard after it.
If the block is longer than that reservation, it is reserved in its own length, with the guard after it.

## Functions

| Function | Result |
| --- | --- |
| `is_beyond` | Refuses a size above `UINT32_MAX`, by its key, because an offset is 32 bits. 1 with a record. |
| `lay_sparse` | Places the indexes of the sparse sets after the entity index. 1 with a record if they are past what an offset of 32 bits reaches. |
| `lay_direct` | Places the direct-indexed bits after the indexes of the sparse sets. 1 with a record if they are past what an offset of 32 bits reaches. |
| `lay_rest` | Places the table directory, the arena and the columns, and gives the block a length: a whole number of the granule, at most 2^32 bytes. 1 with a record if not. |
| `block_lay_out` | Fills the `block_layout_t` of the shared block. 1 with a record if it cannot. |
