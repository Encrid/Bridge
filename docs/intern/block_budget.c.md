# src/block_budget.c

This file checks the budget of a deployment, key by key, before the library makes any sum of it.

## The budget of a deployment

The budget has two tiers, because two things are sized.
The deployment states the shared tier in `bridge_budget_t`, and the library reads it when it creates the world.
One shared block serves each module, so no single module can size it.
The budget comes as a struct, because the library reads no file.
Each module states its own tier when it loads: `docs/intern/module_check_fit.c.md`, *The budget of a module*.

| Key | How many there may be at one time, or how many bytes |
| --- | --- |
| `entities` | Entity indexes. Index 0 is no entity, so `entities` of N holds N - 1 entities. |
| `tables` | Tables: the sets of components that entities carry. |
| `types` | Declared types: the components, and the structs, enums and bitmasks that their fields name. |
| `systems` | Systems and observers registered. |
| `terms` | The terms that one system or observer may have. |
| `modules` | Modules loaded. |
| `sparse_sets` | Components kept in a sparse set, or `BRIDGE_NONE`. |
| `direct_indexed` | Components kept direct-indexed, or `BRIDGE_NONE`. |
| `persistent` | The bytes of the persistent arena. |
| `columns` | The bytes of the rows of every table: its entity ids and its columns. |

`tables` counts what exists at one time, and not what ever existed.
A table lives while it has rows, so the number to state is how many sets of components the entities carry at one time.
`tables` sizes what storage keeps for each table, and the `column_base` of each module.

`types` and `systems` count what exists at one time, across every module.
A type that two modules declare counts once.
With these two keys, everything that storage holds is known and allocated when the world is made, and storage allocates nothing after.
A `declare` or an `add_system` past its key is refused, by name, and the first call of that module fails.

`terms` is a key because storage keeps the terms of a system in a record made with the world, so the record has a length.
No module compiles against the number.

`sparse_sets` and `direct_indexed` are keys because of what those two storage layouts keep for each entity there may be.
A sparse set keeps an index for each entity, and a direct-indexed component keeps a bit for each entity.
How many such components there will be is not known when the world is made, so the deployment states it.
Either may be none, which is `BRIDGE_NONE`. Zero is unset, and the check refuses it.

`modules` bounds how many records of modules exist at one time.
A load takes a record and an unload frees it, and a load with no record left is refused.

Each amount of the budget is fixed for as long as the block exists.
A block handed to a backend cannot move, so no budget grows.

## No defaults

A default is a value that nobody accounted for, so the library takes none.
A key left unset in every source is an error, which names the key at create.
A host may let a deployment state one value for many modules: that value is still stated.
No key of this budget has a floor.
A floor that clamps a value would change what the deployment stated, with nobody told.

## The checks

1. Refuse the first of the eight amounts left at 0: `entities`, `persistent`, `columns`, `modules`, `tables`, `types`, `systems`, `terms`.
2. Refuse `entities`, `persistent` or `columns` above `UINT32_MAX`: an offset is 32 bits.
3. Refuse `sparse_sets` or `direct_indexed` that is neither `BRIDGE_NONE` nor a count from 1 to `UINT32_MAX`.

The first fault ends the check, with one record that names the key.
The other counts are 32 bits wide in `bridge_budget_t`, so no offset can be past them.

## Functions

| Function | Result |
| --- | --- |
| `is_unset` | Refuses an amount left at 0, by its key. 1 with a record. |
| `is_beyond` | Refuses a size above `UINT32_MAX`, by its key, because an offset is 32 bits. 1 with a record. |
| `is_miscounted` | Refuses a count that may be none and is neither a count nor `BRIDGE_NONE`. Zero is unset. 1 with a record. |
| `has_unset` | Refuses a budget that leaves one of its eight amounts unset, at the first. 1 with a record. |
| `block_check_budget` | Refuses a budget with a key unset, a size past what 32 bits reach, or a count that is neither a count nor none. 1 with a record. |
