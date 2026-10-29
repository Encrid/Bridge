# src/storage/storage_apply.h

This header gives the second, the third and the fourth step of a barrier: the room, the observers, and the placement.
A delete at an unload takes the same steps: `docs/intern/storage/storage_husk.c.md`, *A delete at an unload*.

## One path for a destroy

`storage_apply` calls the two steps between the room and the values: `docs/intern/storage/storage_apply.c.md`, *The steps of a barrier*.
`storage_delete_carriers` calls them after its own fold and `reserve_all`, and it writes no value.
Both steps read only what the fold left: `changed`, `folded`, `work` and `attached`.
Thus a destroy keeps, moves and kills the same way at a barrier and at an unload.

## Functions

| Function | Result |
| --- | --- |
| `reserve_all` | The second step of a barrier, of a move and of a delete. Finds a table for each changed entity, then room in it and in each sparse set that it joins, before anything moves: `src/storage/storage_room.h`. |
| `tell_all` | The third step, when there is an observer. For each changed entity that is not left as it was, gives each observer what it is owed. A destroyed entity has no set after. |
| `place_all` | The fourth step. Places each changed entity that is not left as it was, with `place_one`. |
