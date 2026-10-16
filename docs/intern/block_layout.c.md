# src/block_layout.c

This file finds where each part of the shared block begins, how long the block is, and how much address space it needs.

## The parts

| Part | Begins at | Holds |
| --- | --- | --- |
| None | 0 | Eight bytes with nothing in them, so that offset 0 is none and the entity index is aligned. |
| `index` | 8 | The entity index: a `bridge_entity_t` of twelve bytes for each of `[budget] entities`. |
| `sparse` | A multiple of 8 | For each sparse set, its index: a `u32` for each entity. |
| `direct` | A multiple of 8 | For each direct-indexed component, its bits: a bit for each entity, in words of 64 bits. |
| `arena` | A multiple of 8 | The persistent arena: `[budget] persistent` bytes. |
| `columns` | A multiple of 16 | The column region: `[budget] columns` bytes. |

A count of `BRIDGE_NONE` gives no sparse set or no direct-indexed component, and its part is empty.
The length of the block is a whole number of the granule that the backends need.

Each amount of the budget is below 2^32 when this file runs, so no sum can be past 64 bits.

## The limits

Each offset into the shared block is 32 bits, so the layout refuses three things:

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
| `lay_sparse` | Places the indexes of the sparse sets after the entity index. 1 with a record if they are past what an offset of 32 bits reaches. |
| `lay_direct` | Places the direct-indexed bits after the indexes of the sparse sets. 1 with a record if they are past what an offset of 32 bits reaches. |
| `lay_rest` | Places the arena and the columns, and gives the block a length: a whole number of the granule, at most 2^32 bytes. 1 with a record if not. |
| `block_lay_out` | Finds where each part of the shared block begins, and how much address space the block is reserved in. 1 with a record if it cannot. |
