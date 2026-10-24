# tools/deps.py

`deps.py` gives the functions that the built library imports from the system.
With `--check`, it compares them with the list of `docs/extern/lib/bridge.h.md`, *The imports*.

```sh
python tools/deps.py <folder> [--check]
```

The folder is the first argument that is not `--check`: the output of a build, for example
`build/x86_64-windows-gnu`.
`deps.py` reads the first of the files `bridge.dll`, `bridge.so` and `bridge.dylib` of that folder, by name.

## The imports of a library

`imports` finds the format of the library from its first bytes, and reads its imports:

| Format | First bytes | The imports |
| --- | --- | --- |
| PE, of Windows | `MZ` | Each function that the import directory names, with its library in lower case. A function that has only a number is left out. |
| ELF, of Linux | `0x7f`, then `ELF` | Each name of the dynamic symbol table whose section is 0: no section of the library defines it. |
| Mach-O, of macOS | Each other value | Each undefined name of the symbol table, with its first `_` taken away. |

The header of each format gives the type of the processor. `PROCESSORS` maps its number to `x86_64` or `aarch64`.
`deps.py` refuses a library of a different processor.

## Without --check

`deps.py` writes a line for each import: `bridge`, the library of the import, and the function.
An ELF or a Mach-O library names no library for an import, so that column is empty.
The lines come in the order of the libraries, then of the functions.

## With --check

1. `listed` reads the block `text` of `docs/extern/lib/bridge.h.md`, *The imports*.
2. It keeps each row whose first word is a word of the system of the library, as `SYSTEMS` gives them.
3. The words are `windows` and `every`; `linux`, `posix` and `every`; or `macos`, `posix` and `every`.
4. The name of the system and of the processor also counts, for example `windows_aarch64`.
5. The second word of a row is the function: a pattern of `fnmatch.fnmatchcase`. For example, `*` matches any text.
6. `compare` writes each import that no row matches, and each row that no import matches.

If `compare` wrote a line, `deps.py` stops with the status 1.
Else it writes `<count> imports: each is in the list, and each one in the list is imported`.

A folder whose name ends with `-debug` holds a debug build. `deps.py --check` does not check it, and ends with 0.

## Messages

| Message | When |
| --- | --- |
| `an import on <system>_<processor>: <name> is not in the list` | `--check` found an import that no row matches. |
| `an import on <system>_<processor>: <row> is in the list, and nothing imports it` | `--check` found a row that no import matches. |
| `deps.py: the imports of the library and the list differ` | `--check` wrote one of the 2 lines above. The status is 1. |
| `deps.py: name the folder of a build` | No argument names a folder. The status is 1. |
| `deps.py: no library bridge in <folder>` | The folder holds no library. The status is 1. |
| `deps.py: <file> is for a processor that PROCESSORS does not map: 0x<number>` | The header names a different processor. The status is 1. |
| `deps.py: docs/extern/lib/bridge.h.md is not there` | `--check` found no file of the list. The status is 1. |
| `deps.py: docs/extern/lib/bridge.h.md has no block of text under ## The imports` | The file has no list. The status is 1. |
| `a debug build is not what ships: its imports are not held to the list` | `--check` was given a debug build. The status is 0. |

## Limits

No case runs these branches.

- An import by its number only. The built library of Bridge has none.
- An address outside each section of a PE file. Only a broken library has one.

## Functions

| Function | Result |
| --- | --- |
| `text(data, at)` | Gives the text that starts at `at` and ends at the first byte 0. |
| `pe(data)` | Gives the imports of a PE file: pairs of a library and a function. |
| `elf(data)` | Gives the imports of an ELF file, each with an empty library. |
| `macho(data)` | Gives the imports of a Mach-O file, each with an empty library. |
| `imports(path)` | Gives the system, the processor and the imports of a library. |
| `listed(kinds)` | Gives the functions of the rows of the list whose first word is in `kinds`. |
| `compare(asked, known, what)` | Writes each difference of the imports and the list, and gives their count. |
| `main()` | Reads the arguments, and checks or writes the imports. |
