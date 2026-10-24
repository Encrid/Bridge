# src/storage/storage_convert.c

This file does the second half of a move.
It places each entity once, takes its values before it leaves where they were, and writes them where it is now.
The move is in `docs/intern/storage/storage_convert.h.md`, *A move*.

## The values

`storage_convert` allocates `was`, with room for one value of each moved type, one after the other.
If there is no memory for it, it gives `STORAGE_NO_MEMORY`, and changes nothing.
It then folds the move with `storage_fold_moves`.
If that finds no room, it gives `STORAGE_NO_ROOM`, and nothing changed.

Otherwise, for each changed entity, in the order of the fold:

1. `take_values` copies into `was` the value of each moved type that the entity has.
2. `place_one` places the entity.
3. `write_values` writes each value it had as its new type, where the entity now has that type.

A new value is zero, except for the runs of the move, which `write_runs` copies.
The value of a singleton is on index 0.
At the end, storage forgets the fold with `forget_all`: `docs/intern/storage/storage_forget.h.md`.

## The room of a conversion

`storage_convert` takes one block before it folds anything: a place for each move in `had`, and then the bytes of each old value in `was`.
So a conversion that has no memory changes nothing, and any number of moves fits.

## Functions

| Function | Result |
| --- | --- |
| `take_values` | Copies the value of each moved type that the entity at `index` has, one after the other. Writes into `had` where each was, or 0 for a type it does not have. |
| `write_runs` | Writes the value of one moved type as its new type. It copies the runs of bytes of the old value `was` into the new value at `to`. |
| `write_values` | Writes each value that the placed entity had of a moved type, as the type it became. |
| `convert_all` | Places each changed entity once, with its values carried across. |
| `storage_convert` | Writes the rows of components again as other types, all of them or none. Gives an `enum storage_moved`. |
