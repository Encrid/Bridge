# src/storage_move.h

This header moves an entity from one table to another, gives and takes a direct-indexed value, and sets a singleton.

## A row that moves

`move_row` moves the entity at `index` to table `to`, which has room for it.
It adds a row at the end of `to`, and writes each column of that row.
Then it takes the old row out of its table, and records the new place of the entity.
What the entity carried that `to` also holds goes with it, and what it did not carry is zero.
A toggled component is on when the entity gets it, and it keeps its bit when it moves.
Table 0 is no table: a move to 0 only takes the row out, and a move from 0 only adds one.

## A singleton and a direct-indexed value

`set_single` gives a singleton a value of zero, or takes its value.
It sets `layout_moved` of the world, so storage writes for each module again.
The cell where a module reaches the singleton tells whether it has a value.
`give_cell` sets or clears the bit of an entity for a direct-indexed component, in `given` and in the copy for modules.
It makes the cell of the entity zero either way.

## Functions

| Function | Result |
| --- | --- |
| `set_single` | Makes a singleton have a value, zero until one is written, or have none. |
| `give_cell` | Gives the entity at `index` the direct-indexed component of `slot`, or takes it. |
| `carry_value` | Writes one column of the row that an entity moves into, with the bit of a toggled column. |
| `carry_row` | Writes each column of the row that an entity moves into. |
| `move_row` | Moves the entity at `index` to table `to`. Does nothing if the entity is in `to` already. |
