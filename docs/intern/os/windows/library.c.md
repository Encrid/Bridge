# src/os/windows/library.c

This file gives the shared libraries of `src/os/os.h` on Windows: a library opened, looked into and closed.
Each function keeps the contract of `docs/intern/os/os.h.md`, *Functions*.

## Opening

A name with no `/` and no `\` is a bare name: `os_lib_open` gives it to `LoadLibraryA` and the search of Windows.
Windows looks beside a library for what the library needs only when it is asked, and only for a full path in its own form.
So for a path, `os_lib_open` first makes the path full with `GetFullPathNameA`.
Then it opens the library with `LoadLibraryExA` and `LOAD_WITH_ALTERED_SEARCH_PATH`, which asks for that search.

## Limits

The full path must be shorter than `MAX_PATH` characters: a longer path gives `nullptr`.

## Functions

| Function | Result |
| --- | --- |
| `os_lib_open` | Opens a bare name with `LoadLibraryA`, and a path with `LoadLibraryExA` after `GetFullPathNameA`. Gives `nullptr` on failure. |
| `os_lib_get_symbol` | Gives what `GetProcAddress` finds of `name` in the library, or `nullptr`. |
| `os_lib_close` | Closes the library with `FreeLibrary`. A null library does nothing. |
