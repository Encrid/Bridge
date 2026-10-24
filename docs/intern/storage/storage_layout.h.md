# src/storage/storage_layout.h

This header lays out the block of a table: its entity ids, then a column for each type, each at a multiple of 16.
It also asks the column region for a block, and keeps the index of a sparse set and the bits of a toggle in step.

## The layout of a block

For a block of `room` rows, `lay_out` puts the entity ids first: 8 bytes for each row.
It then puts the column of each type of the set, in the order of the type ids, and writes where each begins into `at`.
A column with bytes, and each toggled column, begins at a multiple of 16.
A toggled column has its bits and 16 bytes before it: `docs/intern/storage/storage_bytes.h.md`, *A toggle*.
A tag that is not toggled has no column, and takes no byte.
`lay_out` makes the length of the block a multiple of 16.

## Functions

| Function | Result |
| --- | --- |
| `lay_out` | Gives how long a block of `room` rows of a set is, and writes into `at` where each column begins. |
| `reseat_set` | Writes again, in the index for modules, where the value of each entity of a sparse set is, after the table of the set moved. |
| `move_bits` | Moves the bits of a toggled column to where the new room puts them, after its values, and writes their offset before the column. `from` is where the bits were in the block, or 0 for a new block. |
| `take_block` | Asks the column region for the block that table `t` needs for `room` rows, grown from its block. Gives the block and writes its length. Gives `nullptr`, with nothing changed, if there is no room or the block is longer than `INT32_MAX`. |
