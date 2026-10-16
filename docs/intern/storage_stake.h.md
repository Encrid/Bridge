# src/storage_stake.h

This header tells what a module may destroy, and puts a type in a set or takes it out by one flag.

## What a module may destroy

A module may destroy an entity in which it has a stake.
Nobody has a stake in an id that a block holds and that `spawn` has not handed out: its row is `HELD`.
The entity carries a component that the module may write, or it carries nothing.
A destroy takes each component on the entity with it, whoever declared it, so it is a write to all of them.
A module with a read grant can find entities in its spans, but it may not remove them.

Storage checks the stake at the barrier, where it knows what the entity carries.
It checks against what the entity carried before that barrier.
A destroy that fails the check leaves the entity as it was, and storage counts it in `barred` of the module.
`say_barred` of `src/storage_settle.h` says how many, and the barrier counts as a failure of the module: `docs/intern/schedule.h.md`, *The failure budget*.
`storage_let_write` of `src/storage_slots.c` records each type that a module may write, in the set `writable` of the module.

## Functions

| Function | Result |
| --- | --- |
| `set_write` | Puts a type in a set if `put` is true, and takes it out if not. |
| `has_apart_stake` | Gives whether a module may destroy the entity at `index`, from all that the entity carries. Storage uses it when a type is kept apart from tables. |
| `is_held` | Gives whether the entity at `index` is held: in no table, with its row `HELD`. |
| `has_stake` | Gives whether a module may destroy the entity at `index`, from where the entity is before this barrier moves it. While no type is kept apart, that is what its table holds. |
| `fold_destroy` | Folds a destroy. The entity is gone at this barrier if the module has a stake in it. If not, the destroy is barred and counted against the module. |
