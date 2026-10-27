# src/module/module.h

This header gives what a load, an unload and a reload of a module share.
It gives the checks of a config, the record of a module, its private memory and first call, and how its types and share leave.

## Between phases only

A load, an unload and a reload are legal only between phases, and never while `bridge_run_phase` runs.
All three change storage and the schedule, and only the time between phases makes that safe.
A native module that holds the world can break this rule.
Nothing contains native code, so the rule is a contract and not a guarantee.
Each call has its steps in its own document:

- `docs/intern/bridge/bridge_load.c.md`, *The steps of a load*
- `docs/intern/bridge/bridge_unload.c.md`, *The steps of an unload*
- `docs/intern/bridge/bridge_reload.c.md`, *The steps of a reload*

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
| A system or an observer | Bridge abandons the invocation, counts it and says it. A count that reaches `max_system_failures` stops the module: `src/schedule/schedule_account.c`. |
| `shutdown` | Bridge says the failure, and the teardown continues. A load or a reload that fails after its first call gave 0 runs it too: `docs/intern/module/module_shutdown.c.md`. |
| `reload` | Bridge keeps the old instance if it still exists. If not, the module is stopped, and keeps all that it declared. |

`docs/extern/bridge.h.md`, *Failures*, tells what a stopped module runs and keeps.
`on_error` is stated for each module, and has no default.
`BRIDGE_FAIL` stops the world: `docs/intern/bridge/bridge_stop.c.md`, *A stopped world*.
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
| `w` | The world. |
| `a` | The artifact. |
| `cfg`, `id` | The config of the artifact, and the identity that the config states. |
| `on` | The backend that the artifact names. |
| `demands` | What the config asks that backend to keep. |

### module_private_t

The place of each region in the private memory of a module, at a multiple of 8.
`docs/extern/bridge_abi.h.md`, *The memories*, gives their order.

| Field | Content |
| --- | --- |
| `ring` | The offset of the log ring. |
| `spans` | The offset of the spans. |
| `end` | The end of the spans, which is the length of the regions. |

### module_pointing_t

What a search of the rows of a type looks for: a string that lies in the persistent share of a module.

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
| `module_refuse` | `src/module/module_say.c` | Says why a module is refused: `docs/intern/module/module_say.c.md`, *Functions*. |
| `module_fail` | `src/module/module_say.c` | Writes why a load or reload failed: `docs/intern/module/module_say.c.md`, *Functions*. |
| `module_check_set` | `src/module/module_say.c` | Refuses a key left at zero: `docs/intern/module/module_say.c.md`, *Functions*. |
| `module_check_bound` | `src/module/module_say.c` | Refuses a wrong negative number: `docs/intern/module/module_say.c.md`, *The keys checked one by one*. |
| `module_check_flag` | `src/module/module_say.c` | Refuses an unset or wrong flag: `docs/intern/module/module_say.c.md`, *Functions*. |
| `module_check_flags` | `src/module/module_check_stated.c` | Refuses the flags of a config: `docs/intern/module/module_check_stated.c.md`, *Functions*. |
| `module_check_phases` | `src/module/module_check_phases.c` | Refuses the phase names of a config: `docs/intern/module/module_check_phases.c.md`, *Functions*. |
| `module_check_stated` | `src/module/module_check_stated.c` | Refuses the keys that nobody stated: `docs/intern/module/module_check_stated.c.md`, *Functions*. |
| `module_check_fit` | `src/module/module_check_fit.c` | Refuses the amounts that do not fit: `docs/intern/module/module_check_fit.c.md`, *The budget of a module*. |
| `module_check_identity` | `src/module/module_identity.c` | Refuses a wrong identity: `docs/intern/module/module_identity.c.md`, *Functions*. |
| `module_check_backend` | `src/module/module_check_backend.c` | Refuses what the backend cannot keep: `docs/intern/module/module_check_backend.c.md`, *Functions*. |
| `module_check` | `src/module/module_check.c` | Checks a config, and finds its demands: `docs/intern/module/module_check.c.md`, *Functions*. |
| `module_check_room` | `src/module/module_check.c` | Refuses a load that has no room: `docs/intern/module/module_check.c.md`, *Functions*. |
| `module_find` | `src/module/module_check.c` | Finds a loaded module by its stem: `docs/intern/module/module_check.c.md`, *Functions*. |
| `module_lay_private` | `src/module/module_check_fit.c` | Lays out the private memory: `docs/intern/module/module_check_fit.c.md`, *Functions*. |
| `module_take_private` | `src/module/module_call.c` | Reserves the private memory: `docs/intern/module/module_call.c.md`, *Functions*. |
| `module_record` | `src/module/module_record.c` | Makes the record of a module: `docs/intern/module/module_record.c.md`, *Functions*. |
| `module_drop_systems` | `src/module/module_discard.c` | Takes the systems out of storage: `docs/intern/module/module_discard.c.md`, *Functions*. |
| `module_release_types` | `src/module/module_types.c` | Stops the declarations of a module: `docs/intern/module/module_types.c.md`, *Functions*. |
| `module_round_share` | `src/module/module_share.c` | Gives a share, rounded up: `docs/intern/module/module_share.c.md`, *Functions*. |
| `module_give_share` | `src/module/module_share.c` | Gives a share back: `docs/intern/module/module_share.c.md`, *Functions*. |
| `module_settle_share` | `src/module/module_share.c` | Settles a share at an unload: `docs/intern/module/module_share.c.md`, *Functions*. |
| `module_count_pointing` | `src/module/module_pointed.c` | Counts the rows that hold a string: `docs/intern/module/module_pointed.c.md`, *Functions*. |
| `module_call` | `src/module/module_call.c` | Calls a lifecycle export: `docs/intern/module/module_call.c.md`, *Functions*. |
| `module_make_instance` | `src/module/module_instance.c` | Makes the instance of a module: `docs/intern/module/module_instance.c.md`, *Functions*. |
| `module_start` | `src/module/module_start.c` | Starts a module: `docs/intern/module/module_start.c.md`, *Functions*. |
| `module_shut_down_new` | `src/module/module_shutdown.c` | Runs the `shutdown` of a new instance that fails: `docs/intern/module/module_shutdown.c.md`, *Functions*. |
