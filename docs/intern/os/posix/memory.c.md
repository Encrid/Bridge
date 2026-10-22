# src/os/posix/memory.c

This file gives the address space of `include/os/os.h` on Linux and macOS: a range mapped with no access, opened to reading and writing, and unmapped.
Each function keeps the contract of `docs/extern/os/os.h.md`, *Functions*.

## The range

`os_reserve` maps `reserve` bytes with `PROT_NONE` and the flags `MAP_PRIVATE` and `MAP_ANONYMOUS`.
A touch of a byte with no access faults.
A commit gives the first bytes `PROT_READ` and `PROT_WRITE` with `mprotect`.
Linux counts those bytes against its commit limit then, and `mprotect` can refuse them.
The rest of the range has no access, and Linux counts none of it.
An anonymous mapping holds zeros, so a committed byte is zero until a write.

## Functions

| Function | Result |
| --- | --- |
| `os_reserve` | Maps the range, and commits the first `commit` bytes. Each caller commits more than 0. Gives `nullptr` on failure, with nothing left mapped. |
| `os_commit` | Opens the first `bytes` of the range to reading and writing with `mprotect`. Gives what `mprotect` gives: 0 on success. |
| `os_release` | Unmaps all `reserve` bytes of the range with `munmap`. |
