# src/host/config_names.c

This file lists the distinct names stated under a key.

`names_add` appends a name when the list does not hold it yet.
`add_names` scans one file for entries below the asked prefix and adds the next name of each.
`config_names` runs that scan over every file.

## Functions

| Function | Result |
| --- | --- |
| `names_add` | Adds a name to a `names_t`, without a repeat. |
| `config_names` | Gives the names under a key, gathered from all files. |
