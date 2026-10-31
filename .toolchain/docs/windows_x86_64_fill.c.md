# .toolchain/runtime/windows_x86_64_fill.c

This file gives `memset` to each shared library of `x86_64-windows-gnu`: `.toolchain/docs/windows_x86_64.h.md`.

## The sizes

`memset` writes the byte many times in one word, then writes the word, as in `.toolchain/docs/windows_x86_64_move.c.md`, *The sizes*:

| Size | Stores |
| --- | --- |
| 0 to 7 bytes | `fill_small`: 2 stores of 4 bytes, or 2 stores of 2 bytes, or a store of one byte. |
| 8 to 32 bytes | `fill_words`: the first and the last 8 bytes, and also bytes 8 to 16 and the 16 bytes before the end from 16 bytes. |
| 33 to 511 bytes | `fill_vector`: 16-byte stores of SSE2, then the last 16 bytes. |
| From `SHORT_PATH` | `RtlFillMemory` of Windows. |

## Functions

| Function | Result |
| --- | --- |
| `fill_small` | Writes 0 to 7 bytes from a word of the byte. |
| `fill_words` | Writes 0 to 32 bytes. |
| `fill_vector` | Writes 33 bytes or more with the 16-byte stores of SSE2. |
| `memset` | Writes the low byte of `fill` into `len` bytes from `to`, and gives `to`. |
