# src/os/windows/memory.c

This file gives `include/os/os.h` its address space on Windows: a range reserved, committed and given back.
Each function keeps the contract of `docs/extern/os/os.h.md`, *Functions*.

## The range

`os_reserve` reserves the range with `VirtualAlloc`, `MEM_RESERVE` and `PAGE_NOACCESS`.
A commit calls `VirtualAlloc` again on the start of the range, with `MEM_COMMIT` and `PAGE_READWRITE`.
Windows gives committed pages that hold zeros.
`os_release` frees all of a reservation by its base alone, with `VirtualFree` and `MEM_RELEASE`, so it does not use `reserve`.

## Functions

| Function | Result |
| --- | --- |
| `os_reserve` | Reserves the range, then commits its first `commit` bytes. Each caller commits more than 0. Gives `nullptr` on failure, with nothing left reserved. |
| `os_commit` | Commits the first `bytes` of the range with `VirtualAlloc` and `MEM_COMMIT`. Gives 0 on success. |
| `os_release` | Frees the reservation. |
