# src/host/host_walk.c

This file walks the content paths and offers each file to a callback.

`walk_content` steps over every path of `content.paths`.
`walk_path` offers a lone file, or reads a folder, and prints `cannot look in '<path>'` when the path is neither.
`walk_folder` reads the entries of a folder and skips the ignored ones.
`is_ignored` tests a path against `content.ignore` by its whole form.

## Functions

| Function | Result |
| --- | --- |
| `walk_content` | Offers each content file to the callback. Gives 0, or -1 when a path fails. |
