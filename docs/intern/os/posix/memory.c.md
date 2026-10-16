# src/os/posix/memory.c

This file gives the address space of `src/os/os.h` on Linux and macOS: a range mapped with no access, opened to reading and writing, and unmapped.
Each function keeps the contract of `docs/intern/os/os.h.md`, *Functions*.

## The range

`os_reserve` maps `reserve` bytes with `PROT_NONE` and the flags `MAP_PRIVATE`, `MAP_ANONYMOUS` and `MAP_NORESERVE`.
A touch of a byte with no access faults.
`MAP_NORESERVE` reserves no swap space for the range.
A commit gives the first bytes `PROT_READ` and `PROT_WRITE` with `mprotect`.
An anonymous mapping holds zeros, so a committed byte is zero until a write.

## Functions

| Function | Result |
| --- | --- |
| `os_reserve` | Maps the range, and commits the first `commit` bytes if `commit` is not 0. Gives `nullptr` on failure, with nothing left mapped. |
| `os_commit` | Opens the first `bytes` of the range to reading and writing with `mprotect`. Gives the result of `mprotect`: 0 on success. |
| `os_release` | Unmaps all `reserve` bytes of the range with `munmap`. A null `base` does nothing. |
