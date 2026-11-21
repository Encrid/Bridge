# .toolchain/runtime/outcomes.h

`outcomes.h` writes the outcomes of the probes of `test --branches` into the file of the hits.
The probes of `branches.py` include it at the end of each probed file: `.toolchain/docs/branches.py.md`, *The hits*.
A host of wasm modules includes it to give the outcomes of each instance: *A host of wasm modules*.
Thus during the cases, the file of the hits has 1 way to write: the mapping of the file that `c23_outcomes_keep` makes.

## Functions

| Function | Result |
| --- | --- |
| `c23_outcomes_keep(bytes, start, count)` | Sets the byte `start + i` of the file of the hits to 1 for each `i` below `count` whose byte `bytes[i]` is not 0. It gives the mapped byte `start`. |
| `c23_outcomes_get_file()` | Maps the file that `C23_BRANCHES` names at the first call, and gives the mapping and its size. A later call gives the same mapping. |
| `c23_outcomes_map(path, size)` | Opens the file, maps all of it to read and to write, closes the file, and gives the mapping. It writes the size of the file into `size`. |

`c23_outcomes_keep` writes nothing and gives `nullptr` in these cases:

- `C23_BRANCHES` is not set.
- The file is empty, or the system cannot open it or map it.
- `start + count` is past the end of the file. Thus a host can give it the bytes of a module that it does not trust.

## The mapping

On Linux and on macOS, `c23_outcomes_map` maps the file with `mmap`. On Windows, it maps the file with `CreateFileMappingA` and `MapViewOfFile`.
The header declares these 2 functions of Windows with names of its own, so `windows.h` does not come into a probed file.
`CreateFileMappingA` takes 6 arguments and `MapViewOfFile` takes 5. Each declaration names each argument of its function, with no `...`.
The system fixes these arguments, thus the rule F5 of `docs/contributing/coding_style.md` does not count them.

The mapping stays until the process ends. Thus an outcome stays after `_Exit` or `abort`: `.toolchain/docs/branches.py.md`, *The hits*.
Each file that includes the header gets its own copy of the static variables of `c23_outcomes_get_file`. Thus each file maps the file of the hits 1 time.
The first call keeps the mapping in static variables. Thus a program calls the functions from 1 thread at a time.

## A host of wasm modules

A wasm module has no file and no environment. Thus its probes keep their bytes in its memory, and a host of the module gives them to `c23_outcomes_keep`.
The host also gives the counts of `test --leaks` to `c23_counts_keep` of `counts.h`, and writes the plan of `fail` into the module: `.toolchain/docs/counts.h.md`.

This section states what the host reads and writes. The host runs no code of the module.
A host that includes `outcomes.h` or `counts.h` needs a compiler of C23, for `nullptr`, `bool` and `_Atomic`. On Windows, `outcomes.h` also needs the labels `__asm__` of GNU C, which clang and gcc have.

### The global

A module that a run of `test --branches`, `test --leaks` or `test` with `fail` builds exports the global `c23b_outcomes`.
The global is an `i32` that is not mutable. Its value is the address of the area in the memory that the module exports as `memory`.

A module of a normal build does not export the global. Thus a host does nothing for such a module.
The kind adds the area to each module of the run: `.toolchain/docs/rewrite.py.md`, *The area*. A module with no probes thus has an area with no chunk.

### The area

The area is 7 words. Each word is an unsigned integer of 4 bytes, with the low byte first.

| Offset | Word | Value |
| --- | --- | --- |
| 0 | `version` | 1. A host reads no other version, and writes nothing for it. |
| 4 | `outcomes` | The address of the first chunk of outcomes, or 0. |
| 8 | `outcomes_end` | The address after the last chunk of outcomes, or 0. |
| 12 | `counts` | The address of the first chunk of counts, or 0. |
| 16 | `counts_end` | The address after the last chunk of counts, or 0. |
| 20 | `plan` | The address of the plan of `fail`. |
| 24 | `room` | The size of the plan, in bytes: 256. |

### A chunk of outcomes

Each probed file of the module gives 1 chunk. Each copy of a probed header gives a chunk of its own, with the same `start`.
A chunk starts at an address that is a multiple of 4.

| Offset | Content |
| --- | --- |
| 0 | `bits`: the count of the bytes of outcomes of the file, a word of 4 bytes. |
| 4 | `start`: where these bytes start in the file of the hits, a word of 4 bytes. |
| 8 | `bits` bytes. A byte is 1 for an outcome that an instance reached, else 0. |

The next chunk starts at the first multiple of 4 after the bytes.
A word 0 at the start of a chunk is padding of the linker. The host skips 4 bytes and reads the next word.

### A chunk of counts

Each file of the count of `test --leaks` gives 1 chunk. A chunk starts at an address that is a multiple of 8.

| Offset | Content |
| --- | --- |
| 0 | `slots`: the number of counts, a word of 4 bytes. |
| 4 | `length`: the number of bytes of the path of the file, a word of 4 bytes. |
| 8 | `slots` counts. Each count is a signed integer of 8 bytes, with the low byte first. |
| `8 + 8 * slots` | The path of the file: `length` bytes, then a byte 0. |

The next chunk starts at the first multiple of 8 after the byte 0 of the path.
A word 0 at the start of a chunk is padding of the linker. The host skips 4 bytes and reads the next word.

### The plan

The plan holds 2 texts, each with a byte 0 at its end: the value of `C23_FAIL`, then the value of `C23_FAIL_PART`.
A variable that is not set gives an empty text. An empty plan makes no call fail: `.toolchain/docs/fail.py.md`, *A wasm module*.

### The steps of the host

The host acts only if the module exports `c23b_outcomes`, and only if `version` is 1.
It checks each address and each size that it reads against the committed size of the memory. If a chunk or the plan goes past the committed size, it reads no more of the area.

1. After the start of an instance, and before `init`, the host writes the plan, if `C23_FAIL` is set.
2. The 2 texts and their 2 bytes 0 may not fit in `room`. Then the host writes no plan, and tells it.
3. At the destroy of an instance, before its memory goes, the host reads the chunks of outcomes. It reads them only if `C23_BRANCHES` is set.
4. For each chunk, it calls `c23_outcomes_keep(base + at + 8, start, bits)`. `base + at` is the chunk in the memory of the host.
5. Then the host reads the chunks of counts, if `C23_LEAKS` is set.
6. For each chunk, it copies the counts and the path, and calls `c23_counts_keep(line, path, counts, slots)`.
7. `line` holds `C23_COUNTS_ROOM(length, slots)` bytes.

The host does the steps 3 to 7 at each destroy of an instance, also after a reload and after a trap. An instance that the host does not destroy gives nothing.
