# src/backends/wasm/wasm_instance.c

This file creates and destroys a module instance, gives it the memory it imports, and caps its memory.
An instance carries its own store, context and linker, and a list of the functions found in it.

## Opening a store

`open_store` makes the store, the context and the linker for the module's engine.
A metered module starts with full fuel; a timed module starts with a deadline that never fires until a call arms it.
`give_memory` adopts an imported block into the instance and defines it on the linker by namespace and field.

## Functions

| Function | Result |
| --- | --- |
| `instance_create` | Makes an instance and gives it every imported memory. Gives null with `err` on a failure. |
| `instance_destroy` | Frees the found functions, the linker and the store, then the instance. |
| `instance_set_memory_limit` | Caps the bytes the instance's store may hold. |
