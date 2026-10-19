# src/backends/wasm/wasm_func.c

This file defines the host functions a module imports, instantiates the module, and finds its exported functions.
A module is instantiated once, on the first function looked up in it.

## Defining and finding

`define_func` refuses a function after the module is instantiated, or a signature wider than 16 parameters or 4 results.
It builds the type and defines the import on the linker with `wasm_call_offered`.
`instantiate` links the module once.
`find_export` reads an export, which must be a function of shape `() -> i32`, and else writes why in `err`.

## Functions

| Function | Result |
| --- | --- |
| `define_func` | Defines a host function on the linker. Refuses one that is too wide or too late. |
| `instance_get_func` | Instantiates the module if needed, then finds an exported `() -> i32` function. |
