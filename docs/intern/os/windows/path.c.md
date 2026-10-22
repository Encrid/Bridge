# src/os/windows/path.c

On Windows this file turns a path into its whole form.
`os_path_whole` wraps `_fullpath` and lets it allocate the buffer.

## Functions

| Function | Result |
| --- | --- |
| `os_path_whole` | Gives the full path from `_fullpath`, or null. |
