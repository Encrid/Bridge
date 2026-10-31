# .toolchain/runtime/windows_x86_64_move.c

This file gives `memcpy` and `memmove` to each shared library of `x86_64-windows-gnu`: `.toolchain/docs/windows_x86_64.h.md`.

## The sizes

Two overlapping moves, of the first N bytes and of the last N bytes, cover each size from N to 2N.
Each function chooses its moves by the size:

| Size | Moves |
| --- | --- |
| 0 to 15 bytes | `move_small`: the first and the last 8, 4 or 2 bytes, or the 1 byte. |
| 16 to 64 bytes | `move_vector`: the first and the last 16 bytes, and also bytes 16 to 32 and the 32 bytes before the end above 32 bytes. |
| 65 to 511 bytes | `move_vector`, for `memcpy` only: a loop of 16 bytes, then the first and the last 16 bytes. |
| From `SHORT_PATH` | `RtlMoveMemory` of Windows, for `memcpy`. From 65 bytes for `memmove`. |

## The overlap

The source and the destination of `memmove` can overlap.
Up to 64 bytes, `move_small` and `move_vector` load each byte before they store any byte, so an overlap gives the right bytes.
Above 64 bytes, `memmove` gives the move to `RtlMoveMemory`, which handles an overlap.
The loop of `move_vector` is for `memcpy` alone, whose source and destination do not overlap.

`move_vector` loads the bytes 16 to 32 and the 32 bytes before the end only above 32 bytes, so it never reads outside the source.

## Functions

| Function | Result |
| --- | --- |
| `move_tiny` | Moves 0 to 3 bytes. |
| `move_small` | Moves 0 to 15 bytes. |
| `move_vector` | Moves 16 bytes or more with the 16-byte moves of SSE2. |
| `memcpy` | Copies `len` bytes from `from` to `to`, and gives `to`. |
| `memmove` | Copies `len` bytes from `from` to `to`, also when they overlap, and gives `to`. |
