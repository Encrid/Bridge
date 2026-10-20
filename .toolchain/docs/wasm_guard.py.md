# .toolchain/tools/wasm_guard.py

`wasm_guard.py` refuses a compiled module that imports a memory or a global.
The wasm_module build runs it between the compile and the merge. It does not run alone.

## Functions

| Function | Result |
| --- | --- |
| `leb(data, at)` | Reads an unsigned LEB128 number at `at`. It gives the number and the next offset. |
| `name(data, at)` | Reads a length, then that many bytes. It gives the text and the next offset. |
| `refuse(path, why)` | Writes why a file is refused, and stops with status 1. |
| `guard(path)` | Reads a module, and refuses it when an import breaks a rule. |

## The guard

A module reaches the shared data only through the accessors, which the merge adds: `docs/intern/backends/wasm/sdk/bridge_wasm.wat.md`.
So a compiled module of its own must import no memory and no global.
`guard` reads the import section of the wasm file, and checks each import:

1. If the file has no wasm magic, it stops: `<path> is refused: it is not a wasm module`.
2. It walks the sections, and skips each one that is not the imports.
3. For a function or a table, it steps over the import and goes on.
4. For a memory, it stops: `<path> is refused: it imports the memory '<module>.<field>'; a module reaches shared data only through the accessors`.
5. For a global, it stops: `<path> is refused: it imports the global '<module>.<field>'; a module imports no global`.

A file with no import section passes, since it imports nothing of its own.
