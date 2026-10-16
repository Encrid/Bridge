# src/storage_any.c

This file looks through the values of a type.
It tells whether a value answers a question, and how many toggled rows are off.

## The walk of the values

`storage_has_any` asks the question of each value of the type, and stops at the first true answer:

| Layout | The values it asks |
| --- | --- |
| A singleton | Its one value, while it has one. |
| Direct-indexed | The cell of each entity that has a value, from index 1. |
| A table, a toggle or a sparse set | The rows of each live table whose set holds the type. |

The table of a sparse set holds the type of the set, so the walk of the tables covers the set.

## Functions

| Function | Result |
| --- | --- |
| `has_any_cell` | Gives whether the question is true of a cell of a direct-indexed type, for an entity that has a value. |
| `has_any_row` | Gives whether the question is true of a row of a type in one table. |
| `storage_has_any` | Gives whether `is` answers true for a value of a type. It gives `is` the bytes of the value. |
| `count_off_rows` | Gives how many rows of one table have a toggled type off. |
| `storage_count_off` | Gives how many of the entities that carry a toggled type have it off. Gives 0 for a type of another layout. |
