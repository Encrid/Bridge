# src/host/state/state_list.c

This file takes the list keys of a module: `read`, `write` and `losses`.

`find_names` gathers a grant from the module tables, and stops at the first file that states it.
`permit_in` descends through a file and the files that it names.
`take_losses` copies the `losses` list into the config.
`say_names` prints a grant when the host shows a module. `state_take_list` routes a key to `read`, `write`, or `losses`.

## Functions

| Function | Result |
| --- | --- |
| `state_take_list` | Takes a grant or the losses into the config. Gives true on success. |
