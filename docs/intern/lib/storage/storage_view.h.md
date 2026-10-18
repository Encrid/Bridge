# src/lib/storage/storage_view.h

This header gives which entity is in a row, and what a module gets for a column.
It also reads and switches the bit of a toggled row.

## A tag

A tag is a component with no bytes: `docs/extern/bridge_abi.h.md`, *bridge_kind*, tells how a module declares it and what it gets for it.
Storage keeps it in each storage layout as any other component, with no column, no values and no cell:

| Layout | What the tag is |
| --- | --- |
| A table | Nothing in the block. |
| A sparse set | The entity ids of its block, with no values. |
| Direct-indexed | The bits alone. |
| A singleton | The flag `set`. |

## Functions

| Function | Result |
| --- | --- |
| `get_who` | Gives which entity is in each row of a table: the twin of its block in `who`. |
| `get_handed` | Gives what a module gets for a column: the column, or `BRIDGE_CARRIED` for a tag that is not toggled. While no tag is declared, it gives the column and looks at no type. |
| `get_cell` | Gives what a module gets for the value of row `row` of a sparse set: where the value is, or `BRIDGE_CARRIED` for a tag. |
| `is_on` | Gives whether row `row` of the toggled column at `column` is on, in a block of `room` rows. |
| `turn_row` | Switches a row of a toggled column on or off. `bits` is where the bits of the column are, which `get_bits` gives. |
