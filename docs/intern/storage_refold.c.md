# src/storage_refold.c

This file does the first half of a move.
It folds the moves, and finds where each changed entity goes, with room for it.
The move is in `docs/intern/storage_convert.h.md`, *A move*.

## The fold of a move

For each move, storage visits each entity that carries the old type.
The entity loses the old type, and gets the new type in its place.
A singleton is on index 0, which is no entity, so the new type goes there, with the value of the one carrier.
So for each move to a singleton that has a carrier, storage folds index 0 first.
Then the singleton has its place before its value comes.

After the fold, storage finds a table and room for each changed entity with `reserve_all`, as at a barrier.
If an entity gets the flag `DROPPED`, storage forgets the fold, and the move changes nothing.
`docs/intern/storage_inside.h.md`, *enum folded*, tells what `DROPPED` is.

## Functions

| Function | Result |
| --- | --- |
| `fold_in` | Counts the entity at `index` among those that the move changes, and starts its fold from what it carries. Does nothing the second time. |
| `retype` | Folds one entity that carries a moved type: it loses the old type, and it or index 0 gets the new one. |
| `is_short` | Gives whether a changed entity has the flag `DROPPED`: `docs/intern/storage_inside.h.md`, *enum folded*. |
| `fold_singles` | Folds index 0 first, for each move to a singleton that has a carrier. |
| `storage_fold_moves` | Folds the moves, and finds a table and room for each entity, with nothing moved. Gives false, with the fold forgotten, if it cannot have them. |
