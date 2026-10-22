# src/os/windows/library.c

This file gives `include/os/os.h` its shared libraries on Windows: a library opened, looked into and closed.
Each function keeps the contract of `docs/extern/os/os.h.md`, *Functions*.

## Opening

A name with no `/` and no `\` is a bare name: `os_lib_open` gives it to `LoadLibraryA` and the search of Windows.
Windows looks beside a library for what the library needs only when it is asked, and only for a full path in its own form.
So for a path, `os_lib_open` first makes the path full with `GetFullPathNameA`.
Then it opens the library with `LoadLibraryExA` and `LOAD_WITH_ALTERED_SEARCH_PATH`, which asks for that search.

## Limits

The full path must be shorter than `MAX_PATH` bytes: a longer path gives `nullptr`.
With a code page of one byte for each character, `GetFullPathNameA` gives 0 for such a path.
With a code page such as UTF-8, a path of fewer than `MAX_PATH` characters can have `MAX_PATH` bytes or more.
`GetFullPathNameA` then gives the size that it needs and fills nothing, and `os_lib_open` refuses that too.

## Functions

| Function | Result |
| --- | --- |
| `os_lib_open` | Opens a bare name with `LoadLibraryA`, and a path with `LoadLibraryExA` after `GetFullPathNameA`. Gives `nullptr` on failure. |
| `os_lib_get_symbol` | Gives the result of `GetProcAddress` for `name` in the library: the address, or `nullptr`. |
| `os_lib_close` | Closes the library with `FreeLibrary`. |
