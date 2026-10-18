# src/lib/storage/storage_drop_apart.c

This file drops a component that no table keeps: a direct-indexed one, one of a sparse set, or a singleton.
The policies are in `docs/extern/bridge_abi.h.md`, *bridge_on_unload*.

## An entity at a time

Storage works outside a barrier here, an entity at a time, with the functions of `src/lib/storage/storage_unload.h`.
For each entity that has the component, it writes into `before` what the entity carries.
It writes into `scratch` the same set without the component.
If the policy destroys an entity left with `scratch`, storage destroys it, and its observers hear a leave.
Otherwise storage tells the observers what the entity loses, and takes the component from it.
At the end, storage gives back the table of each sparse set that nothing is left in.

A sparse set loses its entities from its last row, until it has no table or no row.
A singleton is on no entity, so the three policies say one thing: it has no value.
Its observers hear that index 0 loses it.

## Functions

| Function | Result |
| --- | --- |
| `drop_cell` | Drops a direct-indexed component from the entity at `index`. It destroys the entity if the policy says so, or takes its value and tells its observers. |
| `storage_drop_cells` | Drops a direct-indexed component from each entity that has a value of it. |
| `drop_member` | Drops the component of a sparse set from the entity `id`, which is in the set. |
| `storage_drop_set` | Drops a component of a sparse set, an entity at a time, from the last row. |
| `storage_drop_single` | Drops a singleton: it has no value, and its observers hear it. Does nothing for a singleton with no value. |
