# src/lib/storage/storage_cells.c

This file writes the `column_base` of a module, and its `kept` cells.
A module reads them as `docs/extern/bridge_abi.h.md`, *Spans*, tells.

## The cells

`column_base` has a row for each table id, 0 included, and a cell in each row for each type that the module declared.
The cells of a row are in the order the module declared the types.
A cell holds what `get_handed` gives for the column: its offset from the shared base, 0 for no column, or `BRIDGE_CARRIED`.

After `column_base`, `kept` has a cell for each type that the module declared.
It tells where the module reaches a component that no table keeps:

| Layout | The cell of `kept` |
| --- | --- |
| A sparse set | Where the index of the set for modules begins: a cell for each entity. |
| Direct-indexed | Where its cells begin, after the 16 bytes before them. |
| A singleton with a value | Where its value is, or `BRIDGE_CARRIED` for a tag. |
| A table, a toggle, or a singleton with no value | 0. |

## What it writes

When `all` of the publish is set, storage writes the header of the module with `block_write_header` of `src/lib/block/block_module.c`.
It then makes each cell 0, writes the row of each live table, and writes `kept`.
Otherwise it writes the row of each touched table, and nothing else.
A table that storage gave back is touched, and its columns are all 0, so its row becomes 0.

## Functions

| Function | Result |
| --- | --- |
| `write_cells` | Writes the row of `column_base` of table `t`. |
| `get_kept` | Gives where a module reaches a component that no table keeps, as the table above says. |
| `write_every_cell` | Writes the header and the whole `column_base` of a module: each live table, and `kept`. |
| `storage_publish_cells` | Writes the `column_base` of a module: each table when `all` is set, and otherwise the touched ones. |
