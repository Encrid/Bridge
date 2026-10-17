# src/storage_values.c

This file looks through the values of a type.
It gives each value to a function, and tells how many toggled rows are off.

## The walk of the values

`storage_visit_values` gives the bytes of each value of the type to a function:

| Layout | The values it asks |
| --- | --- |
| A singleton | Its one value, while it has one. |
| Direct-indexed | From index 1, the cell of each entity with a value. |
| A sparse set, a table or a toggle | Each row of each live table that holds the type in its set. |

The table of a sparse set holds the type of the set, so the walk of the tables covers the set.

## Functions

| Function | Result |
| --- | --- |
| `read_cells` | Gives the function each cell of a direct-indexed type, for each entity that has a value. |
| `read_rows` | Gives the function each row of a type in one table. |
| `storage_visit_values` | Gives `read` the bytes of each value of a type. |
| `count_off_rows` | Gives how many rows of one table have a toggled type off. |
| `storage_count_off` | Gives how many of the entities that carry a toggled type have it off. Gives 0 for a type of another layout. |
