# src/module_start.c

This file starts a module: it makes the first call into the module, and checks what the module registered.
A load and a reload both start a module with it.

## The steps

`module_start` starts a module in these steps, and stops at the first failure:

1. Make the image, the private memory and the instance: `docs/intern/module_instance.c.md`.
2. Make the first call, `init` for a load or `reload` for a reload.
3. Refuse a loss that the config accepts and that was not made.
4. For a load, check the settings of the module: `docs/intern/config.c.md`, *The steps*. `src/bridge_load.c` writes them later.
5. Find each export that Bridge calls.
6. Check that the spans hold what a barrier writes for the module.

The first call is the one call in which a module can declare and register.
`init` runs under `init_timeout`, and `reload` under `reload_timeout`.
The settings are one value of a struct, so they are written after `init` declared the struct.
A reload does not write the settings again: after `init`, the component is the live state.

## The losses

A loss that the config accepts is a loss that the module makes.
So a name that nothing was lost under is refused, and a list does not stay in a config from one reload to the next.
A load loses nothing, so it states no loss.
What a loss is: `docs/intern/abi_relay.c.md`, *What each change does*.

## The exports

Each export that Bridge calls is found once, at the start, and not at its first frame.
Each system and each observer is an export of its name.
`shutdown` and `reload` are found too: the three lifecycle exports are those of every module, and none is left out to mean something.
The backend sees whether an export has the shape that a call runs, and refuses one that does not: `docs/intern/abi_define.c.md`, *The signatures*.

## The spans

`column_base` has a cell for each component of the module in each table there may be.
So whether the spans of the module hold what a barrier writes is known at the start, and not at some barrier.
The spans need these bytes:

| Part | Bytes |
| --- | --- |
| A query for each system | 8 |
| A list for each observer | 8 |
| A cell of `column_base` for each component in each table id, counting 0 | 4 |
| A cell for each component, where a component not kept in a table is reached | 4 |

The count of table ids is the count that storage has room for: `src/storage_publish.c`.
`check_spans` rounds the sum up to a multiple of 8, as the publish of a barrier does.
So a module that loads also fits at its first barrier.

## Functions

| Function | Result |
| --- | --- |
| `call_first` | Makes the first call into a module, `init` or `reload`, the one call in which it can declare and register. Gives 1 with `err`. |
| `check_losses` | Refuses a loss that was accepted and not made. Gives 1 with `err`. |
| `find_exports` | Finds each export that Bridge calls: each system, each observer, `shutdown` and `reload`. Gives 1 with `err`. |
| `check_spans` | Refuses a module whose spans do not hold what a barrier writes for it. Gives 1 with `err`. |
| `module_start` | Makes the image, the instance and the private memory, and makes the first call. Gives 0 if the module registered and every export it named is there. |
