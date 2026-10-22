# src/host/state/state_key.c

This file takes one scalar key of a module.

`state_find_key` searches the module tables and the manifest, and reports a clash or a missing key.
`put_value` writes the value into the config field by its width, and maps a flag or an error word.
`state_take_key` skips an author key, hands a list key to `state_take_list`, and otherwise stores the scalar.

## Functions

| Function | Result |
| --- | --- |
| `state_find_key` | Gives the value of a key, or null on a problem. |
| `state_take_key` | Takes one key into the config. Gives true on success. |
