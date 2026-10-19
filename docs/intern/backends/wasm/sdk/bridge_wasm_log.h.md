# src/backends/wasm/sdk/bridge_wasm_log.h

`bridge_line_t` is one line of the log, and `bridge_log` writes it.
A module adds a line into its private ring when the line's level is kept.

## Writing a line

`bridge_log` drops a line whose level is above the kept level.
It advances the ring head; a full ring loses the line, and the head still advances past it, counting the loss.
It stages the text in the scratch arena, then fills the record with the place, the system, the observer, the level, and two numbers.

## Functions

| Function | Result |
| --- | --- |
| `bridge_log` | Appends a line to the module's ring when the level is kept. |
