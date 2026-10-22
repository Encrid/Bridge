# src/host/state/state_setting_find.c

This file matches a setting statement and reads a setting from the manifest.

`state_is_setting` tests whether an entry is `module.<identity>.config.<name>` or its `path`.
`state_states_setting` finds such an entry in one file.
`state_is_path` tells whether the found value came as the `path` form.
`state_own_setting` reads `config.<name>` from the manifest and checks it.

## Functions

| Function | Result |
| --- | --- |
| `state_is_setting` | Gives true when an entry is a setting of the module. |
| `state_states_setting` | Gives true when a file states the setting. |
| `state_is_path` | Gives true when the value is the `path` form. |
| `state_own_setting` | Gives the manifest setting as a `said_t`, or the empty result. |
