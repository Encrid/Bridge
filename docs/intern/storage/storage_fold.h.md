# src/storage/storage_fold.h

This header gives the fold: what each entity that a barrier changes is becoming, kept beside where it still is.

## The sets of the fold

The fold keeps two sets for each entity, `work` and `attached`: `docs/intern/storage/storage_inside.h.md`, *storage_t*.
The fold of an entity starts from what the entity carries now.
An entity that storage leaves as it was gets the flag `DROPPED`.
It also gets `SHORT` when columns are short, and not a table.

Such an entity is a failure of the modules that asked its changes: `docs/intern/storage/storage_room.h.md`, *Running out*.
`write_value` of `src/storage/storage_runs.h` counts it, where it writes the value of the change.

## Functions

| Function | Result |
| --- | --- |
| `get_work` | Gives the set of what the entity at `index` is becoming. |
| `get_attached` | Gives the set `attached` of the entity at `index`. |
| `start_fold` | Starts the fold of an entity from what it carries now. |
| `drop_change` | Marks the entity at `index` `DROPPED`, and also `SHORT` with `for_columns`. |
| `is_joining` | Gives whether the entity at `index`, as the fold has it, is to join the sparse set of `slot`. |
