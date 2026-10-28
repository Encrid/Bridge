# src/storage/storage_forget.h

This header ends the work of a barrier, of a move, or of a delete at an unload, on rows.
It forgets the fold, and gives back each table that nothing is left in.

## Functions

| Function | Result |
| --- | --- |
| `unsize_table` | Forgets the room found for a table at this barrier: `incoming` becomes 0, and `sized` false. |
| `forget_changed` | Forgets what the fold decided of each changed entity, and the room found for its table. `changed` is then empty. |
| `forget_sets` | Forgets the room found for the table of each sparse set. |
| `forget_all` | Forgets the fold, and gives back each touched table that has no row. |
