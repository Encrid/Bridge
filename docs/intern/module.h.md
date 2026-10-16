# src/module.h

This header gives what a load, an unload and a reload of a module share.
It gives the checks of a config, the record of a module, its private memory and first call, and how its types and share leave.

## Between phases only

A load, an unload and a reload are legal only between phases, and never while `bridge_run_phase` runs.
All three change storage and the schedule, and only the time between phases makes that safe.
A native module that holds the world can break this rule.
Nothing contains native code, so the rule is a contract and not a guarantee.
Each call has its steps in its own document:

- `docs/intern/bridge_load.c.md`, *The steps of a load*
- `docs/intern/bridge_unload.c.md`, *The steps of an unload*
- `docs/intern/bridge_reload.c.md`, *The steps of a reload*

## When a call fails

A function of a module gives an `int32_t`.
Zero is success, and any other value is a failure: Bridge does not read which value it is.
The module writes the details to its log ring.
A non-zero result and a trap are the same failure, and only the backend tells them apart.
So one policy serves an `init` that gives an error and an `init` that reads out of bounds.

The module tells only that it failed, and never how badly.
The severity is a decision of the deployment: a module that could make its own failure fatal could stop the whole deployment.
What Bridge does depends on the call that failed:

| Call | What Bridge does |
| --- | --- |
| `init` | The module never ran, so the deployment can run without it. `on_error` of the module says what the failure costs. |
| A system or an observer | Bridge abandons the invocation, counts it and says it. A count that reaches `max_system_failures` stops the module: `src/schedule_account.c`. |
| `shutdown` | Bridge says the failure, and the teardown continues. |
| `reload` | Bridge keeps the old instance if it still exists. If not, the module is stopped, and keeps all that it declared. |

A stopped module runs no system and no observer, and its components stay.
`on_error` is stated for each module, and has no default.
`BRIDGE_FAIL` stops the world: `docs/intern/bridge_stop.c.md`, *A stopped world*.
`BRIDGE_SKIP` refuses the load of that one module, and the world continues.

Bridge takes a module out when the module fails, and a failure during that work must not start the work again.
What can fail then is the `shutdown` of the module: Bridge says the failure once, and never calls it again or counts it.
A `reload` that fails is said once too, and is not tried again.

## Constants

| Constant | Value |
| --- | --- |
| `MODULE_SCRATCH_AT` | 128: the offset of the scratch arena in the private memory of a module, after the header. |

## Types

### module_check_t

An artifact that the checks hold before it is touched: all that a load and a reload both refuse from a config alone.

| Field | Content |
| --- | --- |
| `b` | The world. |
| `a` | The artifact. |
| `cfg`, `id` | The config of the artifact, and the identity that the config states. |
| `on` | The backend that the artifact names. |
| `demands` | What the config asks that backend to keep. |

### module_private_t

Where the regions of a module sit in its private memory.
The header comes first, then the scratch arena, the log ring and the spans, each at a multiple of 8.

| Field | Content |
| --- | --- |
| `ring` | The offset of the log ring. |
| `spans` | The offset of the spans. |
| `end` | The end of the spans, which is the length of the regions. |

### module_pointing_t

What a search of the rows of a type looks for: a string that lies in the share of a module of the persistent arena.

| Field | Content |
| --- | --- |
| `m` | The module whose share is searched. |
| `c` | The type whose rows are read. |
| `rows` | How many rows hold such a string. |

### module_call_t

One call of a lifecycle export.

| Field | Content |
| --- | --- |
| `f` | The function that is called. |
| `name` | Its name. |
| `timeout` | Its timeout, in nanoseconds. |

### module_start_t

What a module starts from.

| Field | Content |
| --- | --- |
| `a` | The artifact of the module. |
| `demands` | What its config demands of its backend. |
| `entry` | The first call into it: `init` for a load, and `reload` for a reload. |

## Functions

| Function | Source | Result |
| --- | --- | --- |
| `module_refuse` | `src/module_say.c` | Says that a module is refused, and why, written as `printf` would. Gives 1. |
| `module_fail` | `src/module_say.c` | Writes why the load or the reload of a module failed into `err`, as `printf` would. Gives 1. |
| `module_check_set` | `src/module_say.c` | Refuses a key whose value is zero, which means that nobody stated it. Gives 1 if it refuses. |
| `module_check_bound` | `src/module_say.c` | Refuses a negative number that is not `none`: `BRIDGE_NO_LIMIT` or `BRIDGE_NONE`, the "none" of the key. Gives 1 if it refuses. |
| `module_check_flag` | `src/module_say.c` | Refuses a key that is yes or no and is stated as neither. Gives 1 if it refuses. |
| `module_check_flags` | `src/module_check_stated.c` | Refuses a config with a flag left unset. |
| `module_check_phases` | `src/module_check_fit.c` | Refuses a config whose phase names are unset, mapped twice, or mapped to a phase that the deployment did not declare. |
| `module_check_stated` | `src/module_check_stated.c` | Refuses a config with a policy, a limit or an amount left unset, or stated as a number that is nobody's. |
| `module_check_fit` | `src/module_check_fit.c` | Refuses a config whose amounts do not fit: a wrong log level, too many spawns, regions past 32 bits, or a ring too small. |
| `module_check_backend` | `src/module_check_backend.c` | Refuses a config that states what its backend cannot keep, or that allows what its author says cannot be done. |
| `module_check` | `src/module_check.c` | Refuses all that a load and a reload both refuse before the artifact is touched, and finds the backend and its demands. Gives 1 with a record. |
| `module_check_room` | `src/module_check.c` | Refuses a load of a module that is loaded already, one past `[budget] modules`, or a share that does not fit. Gives 1 with a record. |
| `module_find` | `src/module_check.c` | Gives the loaded module of an identity, whatever its version, or `nullptr`. |
| `module_lay_private` | `src/module_check_fit.c` | Finds where the regions of a module sit in its private memory. Gives 1 if they cannot be laid out in 32 bits. |
| `module_take_private` | `src/module_call.c` | Reserves the private memory of a module and hands it to its backend. Gives 1 with `err`. |
| `module_record` | `src/module_record.c` | Makes the record of a module from its config: all of it, or none. Gives `nullptr` with a record said. |
| `module_drop_systems` | `src/module_discard.c` | Takes the systems and the observers of a module out of storage, and frees them. |
| `module_release_types` | `src/module_types.c` | Stops the declarations of a module, last declared first. The last declarer of a type applies its policy, unless `by_policy` is false. |
| `module_give_share` | `src/module_share.c` | Gives the share of a module back to the persistent arena. |
| `module_settle_share` | `src/module_share.c` | Gives the share of a module back at its unload, unless a row that stays holds a string in it. |
| `module_count_pointing` | `src/module_pointed.c` | Gives how many rows of a type hold a string in the share of a module. |
| `module_call` | `src/module_call.c` | Calls a lifecycle export once, under its timeout. Gives 0 if the export gave 0, and 1 with the reason in `err` if not. |
| `module_make_instance` | `src/module_instance.c` | Compiles the image of a module, reserves its private memory, and makes its instance with the memories it imports and the ABI. Gives 1 with `err`. |
| `module_start` | `src/module_start.c` | Makes the image, the instance and the private memory, and makes the first call. Gives 0 if the module registered and every export it named is there. |
