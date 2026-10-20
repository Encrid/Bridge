# src/backends/wasm/sdk/bridge_wasm_kept.h

Some components live outside the table rows: a singleton, a sparse set, or a direct-indexed column.
This header asks the accessors about them, and reads a table's column base from the private header.

## Functions

| Function | Result |
| --- | --- |
| `bridge_has` | Gives whether a kept component is present for an id. |
| `bridge_base` | Gives where a type's column begins in a table, from `column_base`. |
