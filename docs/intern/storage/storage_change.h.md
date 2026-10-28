# src/storage/storage_change.h

This header gives the first step of a barrier: it folds each queued change into what its entity will carry, and moves nothing.

## The fold

Storage reads a run of a queue: the changes that a module asked in one of its turns, in their order.
At the first change of an entity, storage marks the entity `TOUCHED`, adds it to `changed`, and starts its set from what it carries now.
A destroy goes through `fold_destroy` of `src/storage/storage_stake.h`, which checks the stake of the module, and can leave a husk.
An attach puts the type in the set, and a detach takes it out.
While there is an observer, storage writes the same change into a second set, `attached`.
That set holds each type whose last change at this barrier is an attach.

Storage folds nothing for a change of an entity that is not alive, or that this barrier destroyed already.
A held id is an id of a block that `spawn` has not handed out: its row is `HELD`.
An attach or a detach of a held id counts as a fault of the module.
Storage folds nothing for it: `docs/extern/bridge_abi.h.md`, *attach and detach*.
A destroy of a held id goes on to `fold_destroy`, which refuses it: `docs/intern/storage/storage_stake.h.md`.

`foldable` gives a verdict for each change. The first row that fits the change gives it:

| The change | Verdict |
| --- | --- |
| An attach or a detach of a singleton | `FOLD_APPLY` |
| A change of a dead id, or of an entity that this barrier destroyed | `FOLD_SKIP` |
| An attach or a destroy of a husk | `FOLD_HUSK` |
| A destroy, or a change of an id that `spawn` handed out | `FOLD_APPLY` |
| An attach or a detach of a held id | `FOLD_UNHANDED` |

The fold applies nothing of a change of `FOLD_HUSK`: `docs/intern/storage/storage_husk.h.md`, *A husk in the fold*.
`fold_run` also writes `CHANGE_SKIPPED` into the `op` of that change in the queue, with `skip_change`.
The pass of the values reads the queue again, and there an attach after the destroy of a husk looks like an attach before it.
The mark tells the two apart, and `write_value` passes over the marked change: `docs/intern/storage/storage_runs.h.md`, *Functions*.
A change of `FOLD_SKIP` needs no mark: its entity is dead or destroyed, so its value has no place.

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

The settings of a module are a singleton: `src/config/config.c` gives it its value with `storage_give_single`, outside a barrier.
When its last declarer unloads, a singleton has no value, whatever the policy, and its observers hear a leave.

## Functions

| Function | Result |
| --- | --- |
| `foldable` | Gives the verdict of a change, by the table of *The fold*. |
| `begin_entity` | At the first change of the entity at `index`, counts it among the entities that this barrier changes, and starts its fold. A delete at an unload calls it too. |
| `fold_change` | Folds one change into what its entity will carry, and gives the verdict of `foldable`. After a detach, `end_husk` ends a husk that has nothing left. |
| `fold_run` | Folds one run of the queue of a module, from `at` to `end`. Each change has its value after it, padded to 8 bytes. It marks each change of `FOLD_HUSK` with `skip_change`. |
| `skip_change` | Writes `CHANGE_SKIPPED` into the `op` of the queued change at `at`. |
