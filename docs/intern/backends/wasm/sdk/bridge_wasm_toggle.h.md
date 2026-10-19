# src/backends/wasm/sdk/bridge_wasm_toggle.h

This header reads and writes a toggled column's bits: one bit for each row.
It finds the word of a row, then tests or sets the row's bit.

## Functions

| Function | Result |
| --- | --- |
| `bridge_get_bits` | Gives the words of a toggled column, which sit before its base. |
| `bridge_is_on` | Gives whether a row's bit is set. |
| `bridge_turn` | Sets or clears a row's bit. |
