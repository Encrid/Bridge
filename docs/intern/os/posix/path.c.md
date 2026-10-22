# src/os/posix/path.c

This file resolves a path to its whole form on Linux and macOS.
`os_path_whole` wraps `realpath` and asks it to allocate the result.

## Functions

| Function | Result |
| --- | --- |
| `os_path_whole` | Gives the canonical path from `realpath`, or null. |
