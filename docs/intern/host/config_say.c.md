# src/host/config_say.c

This file prints keys, places, and the reason that a key is refused.

`config_say_key` writes a dotted key, and quotes a name that is not bare.
`config_say_found` prints `not stated: <key>` when nothing states the key, or a clash between two peers.
`config_say_where` gives the file and line of a value, or `its file` for a manifest value.
`config_say_over` prints each other file that also states the key.

## Functions

| Function | Result |
| --- | --- |
| `config_say_key` | Writes a dotted key into a buffer and gives it. |
| `config_say_found` | Gives true when exactly one value is found, else prints the problem. |
| `config_say_where` | Gives the place of a value as text. |
| `config_say_over` | Prints the peer files that the chosen value overrides. |
