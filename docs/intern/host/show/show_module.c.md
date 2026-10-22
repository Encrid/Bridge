# src/host/show/show_module.c

This file prints one module while the host shows a configuration.

`show_module` reads the bytes of a file and its manifest.
It prints `not a module: <file>: <reason>` when the file carries no module.
Otherwise it prints the identity, the backend and the path, then the keys through `config_load` and `state_found`.
A key that cannot be taken raises the failure count.

## Functions

| Function | Result |
| --- | --- |
| `show_module` | Prints a module and its keys. Counts a failure on a bad key. |
