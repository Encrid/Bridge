# src/storage_unload.h

This header gives what an unload and a release share: the work on one entity at a time, outside a barrier.

## Outside a barrier

At an unload or a release, storage destroys or changes one entity at a time, with no fold.
It tells the observers at once, from what the entity carries and what it will carry.
`before` holds what the entity carries, and a second set holds what it will carry once the type goes.
A policy destroys an entity always for `delete`, and for `sweep` when nothing is left.
The policies are in `docs/intern/storage_drop.h.md`, *What unload leaves behind*.

## Functions

| Function | Result |
| --- | --- |
| `destroy_entity` | Destroys the entity at `index`, and tells its observers what it carried. Gives back its table if it was the last row. |
| `sweep_sets` | Gives back the table of each sparse set that nothing is left in, as the end of a barrier does. |
| `is_apart` | Gives whether the entity at `index` is in a sparse set, or has a direct-indexed value. |
| `see_less` | Writes into `before` what the entity at `index` carries, and into `left` the same set without a type. |
| `is_swept` | Gives whether a policy destroys an entity that is left with `left`. |
