# src/host/toml_key.c

This file reads a dotted key of one or more names.

A name is bare or quoted.
`read_name` reads a quoted name through the text reader, and a bare name as the run of letters, digits, underscore and hyphen.
`toml_read_key` reads names while a dot joins them, and extends the key array for each name.

## Functions

| Function | Result |
| --- | --- |
| `toml_read_key` | Reads a dotted key into an array, or refuses a name that is missing. |
