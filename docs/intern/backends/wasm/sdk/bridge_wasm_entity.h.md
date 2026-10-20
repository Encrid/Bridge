# src/backends/wasm/sdk/bridge_wasm_entity.h

This header reads entities: the id at a row, and the place an id stands for.
It calls the accessors, which reach the entity index and the who-array of each table.

## Functions

| Function | Result |
| --- | --- |
| `bridge_get_entity` | Gives the entity at a table and a row. |
| `bridge_find` | Gives an id's table and row, or all ones if it is dead. |
| `bridge_is_alive` | Gives whether an id still names a live entity. |
| `bridge_entity_table` | Gives the table of a packed place. |
| `bridge_entity_row` | Gives the row of a packed place. |
