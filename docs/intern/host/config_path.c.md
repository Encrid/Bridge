# src/host/config_path.c

This file builds and splits the paths of a configuration.

`is_whole` treats a path as whole when it opens with a slash or a backslash, or when its second character is a colon.
`config_path` prepends the folder of a file to a path that is not whole.
`config_go_beside` cuts the main file at its last separator and enters that folder.
`config_get_folder` returns the folder part of a path, up to and including the last separator.

## Functions

| Function | Result |
| --- | --- |
| `config_path` | Joins a folder and a path into fresh memory. |
| `config_go_beside` | Enters the folder of the main file and gives the bare file name, or null. |
| `config_get_folder` | Gives the folder prefix of a path. |
