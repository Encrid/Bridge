# .toolchain/tools/wasm_guard.py

`wasm_guard.py` refuses a compiled wasm module that imports a memory, a global or a tag.
The kind `wasm_module` of the preset `wasm_module` runs it between the compile and the merge: `.toolchain/docs/presets.md`, *wasm_module*.
It takes the path of 1 file, and does not run alone.

## Functions

| Function | Result |
| --- | --- |
| `leb(data, at)` | Reads an unsigned LEB128 number at `at`. It gives the number and the next offset. |
| `name(data, at)` | Reads a length, then that many bytes. It gives the text and the next offset. |
| `refuse(path, why)` | Writes why a file is refused, and stops with status 1. |
| `guard(path)` | Reads a module, and refuses it for each fault of *The guard*. |
| `guard_imports(path, data)` | Walks the sections of a module. It refuses an import that breaks a rule, and a section that ends after the file. |

## The guard

A module reaches the memories of Bridge only through the accessors, which the merge adds.
So a compiled module of its own imports no memory, no global and no tag.
`guard` reads the import section of the wasm file, and checks each import:

1. If the file has no wasm magic, it stops: `<path> is refused: it is not a wasm module`.
2. It walks the sections, and skips each one that is not the imports.
3. For a function or a table, it steps over the import and goes on.
4. For a memory, it stops: `<path> is refused: it imports the memory '<module>.<field>'; a module reaches shared data only through the accessors`.
5. For a global, it stops: `<path> is refused: it imports the global '<module>.<field>'; a module imports no global`.
6. For a tag, it stops: `<path> is refused: it imports the tag '<module>.<field>'; a module imports no tag`.
7. If the file ends before a section or an import ends, it stops: `<path> is refused: it ends inside a section`.

A file with no import section passes, since it imports nothing of its own.
The guard is an early check of the build. BB-Wasm checks each module again when it loads it.
