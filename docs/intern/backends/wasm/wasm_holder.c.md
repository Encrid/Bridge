# src/backends/wasm/wasm_holder.c

Bridge adopts a block of host memory as a module's import, but wasmtime only wraps a block through an instance.
This file synthesises a tiny module that imports and exports one memory, so the adopted block becomes an extern wasmtime can define.

## Building the holder

`write_leb` writes an unsigned LEB128 number.
`build_holder` assembles a module by bytes: a memory of the block's page count, exported as `m`.
`wasm_adopt_into` builds the holder once per engine kind, then instantiates it while the backend points at the block, which makes `make_memory` wrap that block.
It reads the export `m` as the memory extern the caller defines.

## Functions

| Function | Result |
| --- | --- |
| `wasm_adopt_into` | Instantiates the holder over an adopted block and gives its memory extern. |
| `memory_adopt` | Records a block as an adoptable memory. Refuses a block that is not whole 64 KiB pages. |
| `memory_release` | Deletes the holder of every engine kind and frees the block record. |
