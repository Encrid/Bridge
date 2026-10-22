# include/backends/native/bridge_native_log.h

This header gives `bridge_line_t`, one line of the log, and `bridge_native_log`, which writes a line.
A module writes a line into the ring of its private memory, when the line's level is one it keeps.
`bridge_line_t` holds the line's `text` and its `len`, with two numbers `a0` and `a1` a format reads.

## Writing a line

`bridge_native_log` reads the header, finds the ring, and moves the head forward.
A ring that is already full drops the line, and the moved head counts the drop.
It stages the line's text in the arena, then fills the record from the header and the line.

## Functions

| Function | Result |
| --- | --- |
| `bridge_native_log` | Writes a line into the module's ring, when its level is kept. |
