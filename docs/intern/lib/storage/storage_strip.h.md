# src/lib/storage/storage_strip.h

This header makes a table stop carrying a type, where it stands.

## A strip in place

Storage takes the table out of the hash and out of the lists of the systems, and takes the type out of its set.
It lays out what is left with the same room.
Then it moves each column after the one that went down to its new place, the first column first.
It moves the bits of a toggled column before its values, since both move down.
The columns before the one that went stay where they were.
The block is then the same block, shorter: the column region takes back the end that the block no longer needs.

Last, storage puts the table in the lists again.
It also puts the table in the hash if it is to be the table that its set finds.

## Functions

| Function | Result |
| --- | --- |
| `close_bits` | Moves the bits of a toggled column down to where the shorter layout puts them, and writes their offset before the column. |
| `close_column` | Moves one column down to where the shorter layout puts it, with its bits first if it is toggled. |
| `shorten_block` | Lays out what is left of a table, and closes up the columns after the one that went. Gives back what the block no longer needs. The block stays where it is, since a block that shrinks does not move. |
| `strip_table` | Makes a table stop carrying a type, where it stands. With `name`, the table becomes the table that its set finds. |
