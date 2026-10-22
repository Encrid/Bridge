# src/os/posix/folder.c

This file gives the folder calls for Linux and macOS.

`os_folder_set` calls `chdir`. `os_folder_make` calls `mkdir` with mode 0777.
When `mkdir` fails, `os_folder_make` calls `stat` and accepts a folder that is already there.

## Functions

| Function | Result |
| --- | --- |
| `os_folder_set` | Changes the working folder through `chdir`. |
| `os_folder_make` | Makes the folder, or gives 0 when `stat` shows a folder is present. |
