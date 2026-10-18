# src/lib/module/module_check_stated.c

This file refuses each key of the config of a module that nobody stated.
A zero is nobody's value, and so is a negative number that is not the "none" of its key.

## No unaccountable values

Somebody states every value, and Bridge supplies none of its own.
A key left unset is an error that names the key, and Bridge never takes a guess for it.
The keys of the deployment are checked at create, and the keys of a module at its load and at its reload.
A host can layer its sources as it likes, if it hands one stated value for each key.

Unset is zero.
Bridge sees a struct, so a field left at zero is the unset one, and zero is never a legal value for it.
Where "none" is a real answer, it is a value of its own, and there are two, because they say opposite things:

| Value | Meaning | Keys |
| --- | --- | --- |
| `BRIDGE_NO_LIMIT`, -1 | A limit stated as none: the module may use all there is. | `max_system_failures`, `work_limit`, `memory_limit`, `init_timeout`, `reload_timeout`, `shutdown_timeout` |
| `BRIDGE_NONE`, -2 | A reservation stated as none: the module gets nothing. | `spawns`, `persistent`, `changes`, `loss_count` |

Each key takes its own "none".
The other "none", like any other negative number, is refused by the name of the key.
`scratch`, `log_ring` and `spans` have no "none": each is an amount above zero.

A key that is yes or no is an `enum bridge_flag`: `BRIDGE_TRUE` or `BRIDGE_FALSE`.
It is not a bool, because a bool has no third state: its zero is `BRIDGE_UNSET`, and that is refused like any unset key.
The flags are `trusted`, `reloadable`, `hot_reload` and `pinned`.

## The checks

`module_check_stated` refuses, in this order:

1. An `on_error` that is neither `BRIDGE_FAIL` nor `BRIDGE_SKIP`.
2. A limit that is zero.
3. An amount that is zero: `scratch`, `log_ring`, `spans`, `spawns`, `persistent`, `changes` or `loss_count`.
4. A `loss_count` above zero with `losses` unset.
5. A negative number that is not the "none" of its key.
6. A `changes` above `UINT32_MAX`.

The messages of the refusals are in `docs/intern/lib/module/module_say.c.md`, *The keys checked one by one*.

## Functions

| Function | Result |
| --- | --- |
| `module_check_flags` | Refuses a config with a flag left unset, or stated as neither yes nor no. |
| `check_limits_set` | Refuses a limit left at zero. |
| `check_amounts_set` | Refuses an amount left at zero. |
| `check_bounds` | Refuses a negative limit that is not `BRIDGE_NO_LIMIT`, and a negative amount that is not `BRIDGE_NONE`. |
| `module_check_stated` | Refuses a config with a policy, a limit or an amount left unset, or stated as a number that is nobody's. |
