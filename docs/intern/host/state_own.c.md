# src/host/state_own.c

This file reads the author values from the manifest and reports the author keys.

`read_own` reads one manifest value as a TOML value, or keeps it as a text.
`state_find_own` checks that value against the rule of the key.
`state_own_keys` demands `config`, `pinned` and `reloadable`, and prints a missing one.
`say_own_keys` prints the author keys when the host shows a module.

## Functions

| Function | Result |
| --- | --- |
| `read_own` | Reads a manifest value into a `value_t`. Gives true on success. |
| `state_find_own` | Gives the checked author value of a key, or null. |
| `state_own_keys` | Gives true when the manifest states the needed author keys. |
