# src/storage/storage_grow.h

This header gives a table room for more rows: a longer block, and each column moved out to where the new room puts it.

## Growth

A table that needs more rows asks for half as much room again as it had.
So a table that grows a row at a time does not move each time.
Where the column region cannot give that much, the table asks for the rows it needs and no more.
`block_column_regrow` of `src/block/block_column.c` gives the longer block.
It grows the block where it stands when what follows it is free.
Otherwise it takes a new block, and copies the old block to its start.

Measured on a table of 64-byte rows, grown a thousand rows at a time to a hundred thousand:

| Growth | Time for each row | Room not used |
| --- | --- | --- |
| Half again | 55 to 61 ns | 15% |
| Twice | 54 to 62 ns | 28% |
| Only what it needs | 97 to 105 ns | None |

So growth by a share pays, and the smaller share is as fast, with half the room not used.

## Moving the columns

In the longer block, each column is where the old room put it.
`lay_out` writes where the new room puts each column, in `at`.
`move_columns` then moves each column out to that place, the last column first.
So it writes over no column that it did not read yet.

It moves the values of a toggled column first, and then its bits, since both move out.
If the block moved, it copies the ids of the rows to the twin of the new block in `who`.
If the table keeps a sparse set, it writes the index of the set again.

The first block of a table has nothing in it, so nothing moves: `place_columns` only writes where each column is.

## Functions

| Function | Result |
| --- | --- |
| `take_longer` | Asks for a block with half as much room again as the table had, or for `room` rows if that fails. Writes the room it got, and gives the block or `nullptr`. |
| `move_column` | Moves one column out to its place in the new room, with its bits if it is toggled. Writes the new place of the column. |
| `move_columns` | Moves `who` to the new block, and each column out to where the new room puts it, the last first. |
| `place_columns` | Writes where each column of the first block of a table is, and where the bits of a toggled column are. |
| `grow_table` | Makes room for `room` rows in table `t`, or for half as much again as it had. Gives true if the table has the room, and false with nothing changed if it cannot have it. |
