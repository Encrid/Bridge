# src/backends/wasm/wasm_module.c

On this backend a module is compiled wasm, so its image is a wasmtime module.
This file compiles the bytes, lists the imports the module declares, and frees them.

## Compiling

`module_compile` reads the demands on the source: a work limit asks for a metered engine, a timeout asks for an interruptible one.
It compiles the bytes with that engine and keeps the import list.
Empty bytes, or bytes wasmtime cannot compile, are refused with a reason in `err`.
`module_list_imports` writes each import's namespace, field and kind, and gives the total count.

## Functions

| Function | Result |
| --- | --- |
| `module_compile` | Compiles the bytes for the engine the demands pick. Gives the module, or null with `err`. |
| `module_list_imports` | Writes up to `max` imports and gives how many the module declares. |
| `module_destroy` | Deletes the import list and the module, then frees the image. |
