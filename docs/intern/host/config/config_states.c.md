# src/host/config/config_states.c

This file answers whether one file states a plain key or a module key.

`config_states_key` matches an entry whose depth and names equal the asked key.
`config_states_module` searches the three tables of a module: by identity, by backend, then `all`.
`find_in_table` and `is_module_key` match an entry under one table.
Each function reports the value and the line through a `said_t`.

## Functions

| Function | Result |
| --- | --- |
| `config_states_key` | Gives true when the file states the asked key. |
| `config_states_module` | Gives true when the file states the key in a table of the module. |
