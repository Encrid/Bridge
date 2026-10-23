# src/os/posix/library.c

This file gives the shared libraries of `include/os/os.h` on Linux and macOS: a library opened, looked into and closed.
Each function keeps the contract of `docs/extern/os/os.h.md`, *Functions*.

## Opening

`os_lib_open` calls `dlopen` with `RTLD_NOW` and `RTLD_LOCAL`.
`RTLD_NOW` resolves every symbol of the library before `dlopen` returns.
`RTLD_LOCAL` keeps the names of the library to itself: they do not resolve the symbols of a library that opens later.
The loader looks beside a library that a path names, and searches for a bare name, with no flag of this file.

A coverage build of `.toolchain/tools/branches.py` defines `C23B_KEEP_LOADED`.
Then `os_lib_open` adds `RTLD_NODELETE`, so a closed library stays mapped.
Its destructor then writes its branches at the exit of the program.
`.toolchain/docs/branches.py.md`, *The probes*, gives the writer.

## Closing

`os_lib_close` calls `dlclose`.

## Functions

| Function | Result |
| --- | --- |
| `os_lib_open` | Opens the library with `dlopen`. Gives `nullptr` on failure. A coverage build keeps the library mapped. |
| `os_lib_get_symbol` | Gives what `dlsym` finds of `name` in the library, or `nullptr`. |
| `os_lib_close` | Closes the library with `dlclose`. |
