# src/host/main.c

This file is the entry point: it reads the arguments and runs or shows a configuration.

`main` sets the standard output to no buffer, then weighs the arguments.
One argument runs a configuration; `--show-config` with a path shows one.
`config_go_beside` enters the folder of the main file. `take_config` reads, checks and takes the keys of the configuration.
A run then takes the lock and calls `run`; a show calls `show_config` instead.
`say_usage` prints the two usage lines and returns 2.

## Functions

| Function | Result |
| --- | --- |
| `main` | Runs or shows a configuration. Gives 0, 1 or 2. |
