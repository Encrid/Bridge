# src/host/show/show.c

This file prints a configuration for `--show-config`.

## The output

`say_files` prints a line per file, with the file that named it and a mark for the local file.
`say_world` prints `phases`, `threads`, `rounds`, the budget, the backends, the content words, the paths, and the order.
`say_key` and `say_under` print one key, or every key below a prefix.
`show_config` prints the files, then the world, then walks the content once with `show_module`.

## Functions

| Function | Result |
| --- | --- |
| `show_config` | Shows a configuration. Gives 0, or 1 on a refusal. |
