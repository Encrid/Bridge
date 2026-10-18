# src/lib/module/module_say.c

This file says how a module is refused and why a load failed.
It also refuses the keys of a config that are checked one by one.

## Refused or not loaded

Bridge refuses a module at one of two moments, and `on_error` governs only the second.

Before the module exists, its config is held against the deployment and against its backend.
Examples are a key left unset, another ABI, a backend that the deployment does not have, or a capability that the backend cannot keep.
Nothing has begun, and the load gives that the module was refused.
`on_error` does not apply: it is a key of the config that is refused, and it can be the key that is wrong.
`module_refuse` says this at once, as an error about no module: "module 'x' is refused: why".

Once the module exists, a refusal takes the path of a failed `init`, because it is a failure to load.
Examples are a grant denied, a descriptor that does not match, an undeclared phase, a reserved system name and an order that is not stated.
`module_fail` writes the reason into the error of the backend, and the caller says "module 'x' did not load: why".
So the record tells which moment it was.
The records go to the sink of the host: `docs/intern/lib/say/say.c.md`, *The sink*.

## The keys checked one by one

| Function | Passes | Refusal |
| --- | --- | --- |
| `module_check_set` | A value that is not zero. | "key is unset" |
| `module_check_bound` | A value above zero, or the "none" of the key. | "key is n, which is neither a limit nor `BRIDGE_NO_LIMIT`", or "an amount nor `BRIDGE_NONE`" |
| `module_check_flag` | `BRIDGE_TRUE` or `BRIDGE_FALSE`. | "key is unset" for `BRIDGE_UNSET`, and "key is n, which is neither `BRIDGE_TRUE` nor `BRIDGE_FALSE`" for another number |

Why each key must be stated is in `docs/intern/lib/module/module_check_stated.c.md`, *No unaccountable values*.
The reason of a refusal is cut at 511 characters.

## Functions

| Function | Result |
| --- | --- |
| `module_refuse` | Says that a module is refused, and why, written as `printf` would. Gives 1. |
| `module_fail` | Writes why the load or the reload of a module failed into `err`, as `printf` would. Gives 1. |
| `module_check_set` | Refuses a key whose zero means that nobody stated it. Gives 1 if it refuses. |
| `module_check_bound` | Refuses a negative number that is not `none`. Gives 1 if it refuses. |
| `module_check_flag` | Refuses a key that is yes or no and is stated as neither. Gives 1 if it refuses. |
