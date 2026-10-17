# src/block_budget.c

This file checks the budget of a deployment, key by key.
Each amount must be set, and each count that may be none must be a count or none.

## The budget of a deployment

The budget has two tiers, because two things are sized.
The deployment states the shared tier in `bridge_budget_t`, and the library reads it when it creates the world: `docs/extern/bridge.h.md`, *bridge_budget_t*.
One shared block serves each module, so no single module can size it.
The budget comes as a struct, because the library reads no file.
Each module states its own tier when it loads: `docs/intern/module_check_fit.c.md`, *The budget of a module*.

`tables` counts what exists at one time, and not what ever existed.
A table lives while it has rows, so the number to state is how many sets of components the entities carry at one time.
`tables` sizes what storage keeps for each table, and the `column_base` of each module.
In `columns`, the block of a table also holds the ids of its entities: `docs/intern/storage_layout.h.md`, *The layout of a block*.

`types` and `systems` count what exists at one time, across every module.
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
2. Refuse `sparse_sets` or `direct_indexed` that is neither `BRIDGE_NONE` nor a count from 1 to `UINT32_MAX`.
3. Refuse `types` past `UINT32_MAX` once rounded up to a multiple of 64. Storage keeps a bit for each type.
4. Refuse `tables`, `sparse_sets` and `types` whose sum and 1 pass `UINT32_MAX`. The column region keeps a range for each.

The first fault ends the check, with one record that names the key.
Checks 3 and 4 add in 64 bits, and bound the counts that storage holds in 32 bits.
The bounds of the offsets are in one place: `docs/intern/block_layout.c.md`, *The limits*.

## Functions

| Function | Result |
| --- | --- |
| `is_unset` | Refuses an amount left at 0, by its key. 1 with a record. |
| `is_miscounted` | Refuses a count that may be none and is neither a count nor `BRIDGE_NONE`. Zero is unset. 1 with a record. |
| `is_too_many` | Refuses `types`, `tables` and `sparse_sets` whose counts storage cannot hold in 32 bits. 1 with a record. |
| `has_unset` | Refuses a budget that leaves one of its eight amounts unset, at the first. 1 with a record. |
| `block_check_budget` | Refuses a budget with a key unset, or a count that is neither a count nor none. 1 with a record. |
