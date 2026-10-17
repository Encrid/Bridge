# src/storage_change.h

This header gives the first step of a barrier: it folds each queued change into what its entity will carry, and moves nothing.

## The fold

Storage reads a run of a queue: the changes that a module asked in one of its turns, in their order.
At the first change of an entity, storage marks the entity `TOUCHED`, adds it to `changed`, and starts its set from what it carries now.
A destroy goes through `fold_destroy` of `src/storage_stake.h`, which checks the stake of the module.
An attach puts the type in the set, and a detach takes it out.
While there is an observer, storage writes the same change into a second set, `attached`.
That set holds each type whose last change at this barrier is an attach.

Storage folds nothing for a change of an entity that is not alive, or that this barrier destroyed already.
A held id is an id of a block that `spawn` has not handed out: its row is `HELD`.
It is no entity, so storage folds no attach and no detach of it: `docs/extern/bridge_abi.h.md`, *attach and detach*.
A destroy of a held id goes on to `fold_destroy`, which refuses it: `docs/intern/storage_stake.h.md`.

## A singleton

A singleton is a component of one value: `docs/extern/bridge_abi.h.md`, *Storage layouts*.
Its block of the column region has the size of the component, and storage takes it at `declare`.
A `declare` that finds no room for the block fails.
It has a value or it has none: it cannot have two.

Each entry that gives or takes a component names an entity, and the id 0 is no entity.
`destroy` refuses the id 0 at the entry, so no destroy reaches the singletons.
So an attach or a detach of a singleton has the id 0.
The fold folds it as the one thing that index 0 carries.
Storage applies the change at the barrier with the others, in the order it was asked.
It tells the observers, and their record has the id 0.

A singleton that gets a value is zero, except for what the attach writes.
After that, a module reads the value where it is, and a system with a term `BRIDGE_WRITE` writes its fields there: `docs/extern/bridge_abi.h.md`, *Storage layouts*.
No observer hears such a write: `docs/extern/bridge_abi.h.md`, *observe*.

The settings of a module are a singleton: `src/config.c` gives it its value with `storage_give_single`, outside a barrier.
When its last declarer unloads, a singleton has no value, whatever the policy, and its observers hear a leave.

## Functions

| Function | Result |
| --- | --- |
| `is_foldable` | Gives whether a change is of something that exists. A change of a singleton must have the id 0. Any other change must name an entity that is alive and that this barrier did not destroy. |
| `begin_entity` | At the first change of the entity at `index`, counts it among the entities that this barrier changes, and starts its fold. |
| `fold_change` | Folds one change into what its entity will carry. |
| `fold_run` | Folds one run of the queue of a module, from `at` to `end`. Each change has its value after it, padded to 8 bytes. |
