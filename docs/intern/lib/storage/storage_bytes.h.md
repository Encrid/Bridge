# src/lib/storage/storage_bytes.h

This header copies a value or makes it zero.
It also gives where a column and the bits of a toggled column begin in a block.

## A toggle

A toggled component is a column of its table, with a bit for each row: `docs/extern/lib/bridge_abi.h.md`, *Storage layouts*.
It is part of the archetype of the entity, as a component in a table is.
A system that states it matches its table, because each row carries it, on or off.
The code of the module tests whether a row is on, and the value of a row that is off stays where it is.

The bits are in the block of the table, before the column:

- First, a 64-bit word for each 64 rows. Row `r` is bit `r & 63` of word `r >> 6`.
- Then 16 bytes. The first 4 hold the offset of the bits from the shared base.
- Then the values.

A module that has the column, from a span or from `column_base`, finds the bits with one load, at the column less 16.
The bits past the last row of the table are of no row.

`docs/extern/lib/bridge_abi.h.md`, *Storage layouts*, tells how a system turns a row on or off, and what an attach does to the bit.
The schedule never runs such a system beside one that reads or writes the component, so no two systems write one word at once.
The write is no call, and it waits for no barrier.
A row that moves, to another table or in its table, takes its bit with it: `carry_value` of `src/lib/storage/storage_move.h`, and `fill_row` of `src/lib/storage/storage_row.h`.

## Copies of values

A value is one or two words more often than not.
A copy of a length that the compiler cannot see is a call, and storage copies a value for each row that moves.
So `copy_bytes` and `clear_bytes` do the two most common sizes, 4 and 8 bytes, with a length the compiler can see.

## Functions

| Function | Result |
| --- | --- |
| `copy_bytes` | Copies a value of `size` bytes. |
| `clear_bytes` | Makes a value of `size` bytes zero. |
| `pad_column` | Gives a count of bytes, rounded up to a multiple of `COLUMN`. |
| `get_bits_len` | Gives how many bytes the bits of a toggled column of `room` rows take. That is a word for each 64 rows, rounded up to a multiple of 16. |
| `get_bits` | Gives where the bits of the toggled column at `column` are, in a block of `room` rows: before the 16 bytes before the column. |
