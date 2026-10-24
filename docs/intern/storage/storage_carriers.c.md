# src/storage/storage_carriers.c

This file visits each entity that carries a type, in each storage layout, and counts them.

## The visit

| Layout | The entities it visits |
| --- | --- |
| A singleton | Index 0, which is no entity, while the singleton has a value. |
| Direct-indexed | Each entity that has a value, from index 1. |
| A table, a toggle or a sparse set | The entity of each row of each live table whose set holds the type. |

A type of a sparse set is in the table of that set, so the walk of the tables finds its entities.

## Functions

| Function | Result |
| --- | --- |
| `visit_given` | Visits each entity that has a value of a direct-indexed component. |
| `visit_rows` | Visits the entity of each row of one table. |
| `storage_visit_carriers` | Calls `visit` for each entity that carries a type. |
| `count_one` | Adds 1 to the count that its context points to. |
| `storage_count_carriers` | Gives how many entities carry a type. It visits them with `count_one`. |
