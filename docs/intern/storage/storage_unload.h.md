# src/storage/storage_unload.h

This header gives what an unload and a release share: the work on one entity at a time, outside a barrier.

## Outside a barrier

At a strip, a sweep or a release, storage destroys or changes one entity at a time, with no fold.
It tells the observers at once, from what the entity carries and what it will carry.
`before` holds what the entity carries, and a second set holds what it will carry once the type goes.
A sweep destroys an entity when nothing is left on it.
A husk that is left with nothing goes under each policy: `docs/intern/module/module_types.c.md`, *The policies at unload*, tells why.

The policies are in `docs/extern/bridge_abi.h.md`, *bridge_on_unload*.
A delete works through a fold, as a barrier does, and not here: `docs/intern/storage/storage_husk.c.md`, *A delete at an unload*.

## Functions

| Function | Result |
| --- | --- |
| `destroy_entity` | Destroys the entity at `index`, and tells its observers what it carried. Gives back its table if it was the last row. The place of a live entity names 0 or a live table. |
| `sweep_sets` | Gives back the table of each empty sparse set, as the end of a barrier does. |
| `see_less` | Writes into `before` what the entity at `index` carries, and into `left` the same set without a type. |
| `is_swept` | Gives whether the entity at `index` goes with the set `left`. It goes if `left` is empty, and the policy is `sweep` or the entity is a husk. |
