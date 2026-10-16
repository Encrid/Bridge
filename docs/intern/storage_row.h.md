# src/storage_row.h

This header adds a row at the end of a table, and takes a row out with the last row moved into its place.

## A row

A row holds the id of its entity, at the start of the block, and a value in each column.
Storage writes the id in two places: in `who`, which it reads, and in the block, which is a copy for modules.
A row that leaves takes the last row into its place, column by column, with the bit of each toggled column.
The entity of the moved row then gets its new place: in the entity index, or in the indexes of its sparse set.
Each change marks the table touched.

## Functions

| Function | Result |
| --- | --- |
| `place_row` | Records that the entity at `index` is in row `row` of table `t`, in `places` and in the entity index that modules read. |
| `seat_row` | Records that the entity at `index` has row `row` of the table of a sparse set, in `where` and in the index that modules read. |
| `append_row` | Adds a row at the end of table `t`, which has room, for the entity at `index`. Writes its id, and gives the row. |
| `fill_row` | Copies the last row of a table into row `row`, a column at a time, with the bit of each toggled column. |
| `remove_row` | Takes row `row` out of table `t`. The last row moves into its place, and its entity gets its new place. |
