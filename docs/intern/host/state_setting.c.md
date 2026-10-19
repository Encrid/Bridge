# src/host/state_setting.c

This file takes the settings that a module declares.

`split_names` splits the manifest `settings` word into names.
`state_settings` refuses a setting that has no component, and takes each declared name.
`take_setting` looks a setting up in the files or the manifest, then flags a clash or an absent value.
`put_setting` writes the setting as a string, an integer, a float, or a boolean, and resolves a path value.

## Functions

| Function | Result |
| --- | --- |
| `state_settings` | Fills the settings array. Gives true when every setting is taken. |
