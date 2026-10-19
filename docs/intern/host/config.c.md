# src/host/config.c

This file opens a configuration: it reads the main file and then the local file.

`config_read` starts an empty `config_t` and takes the main file through `config_take_file`.
`take_local` looks for `<name>.d/local.toml` beside the main file and adds it when it is present.
The local file sits last and overrides the other files.

## Functions

| Function | Result |
| --- | --- |
| `config_read` | Reads the main file and the local file. Gives true on success. |
