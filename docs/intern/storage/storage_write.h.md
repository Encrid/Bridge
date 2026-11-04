# src/storage/storage_write.h

This header writes the values that the changes carry.
The barrier writes them at its fifth step, after each changed entity moved to its table: `docs/intern/storage/storage_apply.c.md`, *The steps of a barrier*.

## Functions

| Function | Result |
| --- | --- |
| `is_written` | Gives whether a change is an attach whose value storage writes. The barrier changed its entity, did not destroy it or leave it as it was, and the id has its generation. |
| `write_value` | Passes over a change of `CHANGE_SKIPPED`. Writes the value of each other change where its entity has the type. For an entity left as it was, it counts the change for the module. `left[0]` counts for want of a table, and `left[1]` for want of columns. |
