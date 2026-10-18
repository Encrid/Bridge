# src/lib/storage/storage_held.c

This file ends the hold on the ids that the modules handed out.

## The ids that a block holds

`storage_reserve` gives each id of a block the row `HELD`, and `spawn` hands the ids out during a phase.
`spawn` runs on the thread of its module, so it does not write storage.
At the start of the next barrier, `storage_hand_out` gives the row 0 to each id that a module handed out.
It also writes the id into the entity index for modules, which showed nothing of it while it was held.
Then the barrier folds the destroys, and bars the destroy of an id that is still `HELD`.
The module that holds such an id can still hand it out.

## Functions

| Function | Result |
| --- | --- |
| `storage_hand_out` | Ends the hold on each id of `ids`: an id with the row `HELD` gets the row 0, and its entry in the entity index. |
