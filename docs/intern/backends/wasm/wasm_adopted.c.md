# src/backends/wasm/wasm_adopted.c

This file builds the wasmtime engines and routes every linear memory wasmtime asks for.
An engine either wraps a block Bridge adopted or hands the module a fresh own memory.

## Making an engine

`make_engine` configures one engine for a kind: fuel on when metered, epoch interruption on when timed.
It fixes the reservation at 4 GiB and the guard at 32 MiB, forbids the memory to move, and sets the memory creator.
`wasm_get_engine` makes the engine for a kind on first need and keeps it.

## Routing a memory

`make_memory` is the memory creator wasmtime calls.
It refuses a request for more address space than `memory_get_shape` states.
With no block to adopt it calls `wasm_make_own`.
With a block it checks the size matches the import, then wraps the block through `get_adopted`, which never grows.

## Functions

| Function | Result |
| --- | --- |
| `wasm_get_engine` | Gives the engine for a kind, and makes it on first need. |
