# src/backends/wasm/wasm_fail.c

This file turns a wasmtime error or trap into one line of `err->text`, then frees it.
It keeps the reason a module gives and drops wasmtime's own stack prefix.

## Finding the reason

`find_reason` scans the message for the last `wasm trap: ` marker and takes the text after it, up to the newline.
It caps the reason at 200 bytes, so one line of `err->text` holds it.
`wasm_fail` reads the message from the error or the trap, writes `what` and the reason, then frees all three.

## Functions

| Function | Result |
| --- | --- |
| `wasm_drop` | Deletes a wasmtime error, if there is one. |
| `wasm_fail` | Writes `what` and the module's reason into `err`, frees the inputs, and gives -1. |
