# .toolchain/runtime/windows_x86_64_compare.c

This file gives `memcmp` and `strlen` to each shared library of `x86_64-windows-gnu`: `.toolchain/docs/windows_x86_64.h.md`.
The start-up code needs them only for short texts, so each one is a plain loop.

## Functions

| Function | Result |
| --- | --- |
| `memcmp` | Compares `len` bytes as unsigned bytes. Gives 0 if they are the same, else the difference of the first bytes that differ. |
| `strlen` | Gives the number of bytes of `text` before its byte 0. |
