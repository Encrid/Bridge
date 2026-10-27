# src/storage/storage_stake.h

This header tells what a module may destroy, and puts a type in a set or takes it out by one flag.

## What a module may destroy

A module may destroy an entity in which it has a stake.
Nobody has a stake in a held id: `docs/intern/storage/storage_change.h.md`, *The fold*.
The entity carries a component that the module may write, or it carries nothing.
A destroy removes each component of the entity but those of `husk_types`, whoever declared them.
So it is a write to all of them.
A module with a read grant can find entities in its spans, but it may not remove them.

Storage checks the stake at the barrier, where it knows what the entity carries: `docs/extern/bridge_abi.h.md`, *destroy*.
With the stake, `fold_husk` decides whether the entity goes or stays as a husk: `docs/intern/storage/storage_husk.h.md`, *A husk in the fold*.
A destroy that fails the check leaves the entity as it was, and storage counts it in `barred` of the module.
`say_barred` of `src/storage/storage_settle.h` says how many, and the barrier counts as a failure of the module: `docs/intern/schedule/schedule.h.md`, *The failure budget*.
`storage_let_write` of `src/storage/storage_slots.c` records each type that a module may write, in the set `writable` of the module.

## Functions

| Function | Result |
| --- | --- |
| `set_write` | Puts a type in a set if `put` is true, and takes it out if not. |
| `has_apart_stake` | Gives whether a module may destroy the entity at `index`, from all that the entity carries. Storage uses it when a type is kept apart from tables. |
| `is_held` | Gives whether the entity at `index` is held: in no table, with its row `HELD`. |
| `has_stake` | Gives whether a module may destroy the entity at `index`, from where the entity is before this barrier moves it. With no type kept apart, that is what its table holds. |
| `fold_destroy` | Folds a destroy. If the module has a stake in the entity, `fold_husk` makes the entity gone or a husk at this barrier. If not, the destroy is barred and counted against the module. |
