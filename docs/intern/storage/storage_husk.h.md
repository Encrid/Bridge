# src/storage/storage_husk.h

This header gives what a fold does with a husk, at a barrier and at a delete of an unload.
`docs/intern/storage/storage_inside.h.md`, *A husk*, tells where storage keeps the mark of a husk.

## A husk in the fold

The fold takes an entity for a husk from the destroy that makes it one.
`is_husk` gives true for an entity whose bit in `husks` is set, and for an entity that the fold marked `HUSKED` at this barrier.
Thus a change that follows the destroy in the same barrier finds a husk.

- `fold_destroy` calls `fold_husk` for a destroy that the module may make: `docs/intern/storage/storage_stake.h.md`, *What a module may destroy*.
- `fold_husk` keeps in the fold of the entity only the types of `husk_types`. If no type stays, the entity is `DESTROYED`, else `HUSKED`.
- `foldable` gives `FOLD_HUSK` when a module attaches to a husk or destroys it, and the fold applies nothing of it: `docs/intern/storage/storage_change.h.md`, *The fold*.
- A detach of a husk applies, as for any other entity. Then `end_husk` marks `DESTROYED` a husk whose fold holds no type.
- Only a module that may write a type of `husk_types` can ask that detach, since the entry checks the grant: `docs/intern/abi/abi_calls.c.md`, *attach and detach*.

A delete at an unload folds each carrier with `fold_husk` too: `docs/intern/storage/storage_husk.c.md`, *A delete at an unload*.

## The rest of the barrier

The other steps treat a `HUSKED` entity as they treat any entity that changed, with the set that the fold left:

- Room: storage finds the table of the types that the husk keeps, or no table, and a row in it. Without room, the entity stays as it was: `docs/intern/storage/storage_room.h.md`, *Running out*.
- Observers: each observer compares what the entity carried with what the husk keeps: `docs/intern/storage/storage_tell.h.md`, *What an observer is owed*.
- Placement: `place_one` sets the bit of the entity in `husks`, and then moves it to its table.
- Values: an attach of a type of `husk_types` before the destroy writes its value into the husk. Storage writes no value of a change of `CHANGE_SKIPPED`.

A husk that is `DESTROYED` goes as any destroyed entity: `kill_entity` clears its bit, and its index goes back to the free list.

## The cost

The fold reads one bit of `husks` for each change of a live entity, beside its flags in `folded`.
A destroy masks the set of its entity with `husk_types`, a word at a time.
While `husk_types` is empty, the mask leaves nothing, and the entity is destroyed as before.

## Functions

| Function | Result |
| --- | --- |
| `is_husk` | Gives whether the entity at `index` is a husk: its bit in `husks` is set, or the fold marked it `HUSKED`. |
| `fold_husk` | Keeps in the fold of the entity at `index` only the types of `husk_types`, with `set_mask`. Then marks the entity `HUSKED` if a type stays, and `DESTROYED` if none does. |
| `end_husk` | Marks the entity at `index` `DESTROYED` if it is a husk and its fold holds no type. |
