# src/backends/wasm/wasm_offered.c

This file wraps a host function Bridge offers a module, so wasmtime can call it with raw arguments.
It reads the arguments by their kinds, calls the function, and writes the results back.

## Calling a host function

`wasm_make_offered` copies the function, its user data, and the kinds of its parameters and results.
`read_args` reads each raw argument as an `i32` or an `i64`; `write_results` writes each result back the same way.
`wasm_call_offered` is the trampoline wasmtime calls: a function that refuses traps the module with `refused by Bridge`.
`wasm_make_types` builds a wasmtime value-type list from the backend kinds.

## Functions

| Function | Result |
| --- | --- |
| `wasm_make_offered` | Gives a record that holds a host function and its signature. |
| `wasm_call_offered` | Reads the arguments, calls the function, and writes the results or traps. |
| `wasm_make_types` | Builds a wasmtime value-type list from `i32` and `i64` kinds. |
