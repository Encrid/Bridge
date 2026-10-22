# include/backends/wasm/bridge_wasm_toggle.h

This header reads a toggle's row bit and writes it, by slot, table and row.
The accessors find the bits word below the column, then test or change the one bit.

## Functions

| Function | Result |
| --- | --- |
| `bridge_is_on` | Gives whether the bit of a row is set. |
| `bridge_turn` | Sets or clears the bit of a row. |
