# src/os/windows/folder.c

This file holds the folder calls for Windows.

`os_folder_set` uses `_chdir`. `os_folder_make` uses `_mkdir`.
If `_mkdir` does not succeed, `os_folder_make` reads `_stat` and keeps a directory that exists.

## Functions

| Function | Result |
| --- | --- |
| `os_folder_set` | Enters the working folder with `_chdir`. |
| `os_folder_make` | Creates the directory, or gives 0 when `_stat` reports `_S_IFDIR`. |
