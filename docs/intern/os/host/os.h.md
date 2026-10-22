# src/os/host/os.h

This header declares the four calls that depend on the platform.
The build compiles one folder under `src/host/os` for each target.
`src/host/os/posix` serves Linux and macOS, and `src/host/os/windows` serves Windows.

## Functions

| Function | Result |
| --- | --- |
| `os_folder_set` | Moves the process into a folder. |
| `os_folder_make` | Makes a folder, or accepts one that is already present. |
| `os_path_whole` | Gives the absolute form of a path in fresh memory. |
| `os_lock` | Opens a file and asks for an exclusive lock that does not block. |
