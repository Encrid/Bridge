# src/host/config/config_content.c

This file gathers the content paths and reads `content.load`.

`collect` reads `content.paths` or `content.ignore` from every file and merges the lists.
`add_path` drops a path whose whole form is already present.
`config_content` fills the path and ignore arrays, and prints `not stated: content.paths` when none exist.
`config_load` reads `content.load.<identity>`, and falls back to `content.load.all`.

## Functions

| Function | Result |
| --- | --- |
| `config_content` | Fills the content paths. Gives true when at least one path exists. |
| `config_load` | Gives 1 to load, 0 to leave, or -1 on a problem. |
