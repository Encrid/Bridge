# src/storage_drop.h

This header declares what the files that drop a component share.
A component is dropped when nobody declares it any more, by the policy that its last declarer stated.

## What unload leaves behind

An unload drops the components that only the unloading module declares, and keeps the shared ones.
The author of each component states at `declare` what happens to the entities that carry it:

| Policy | What storage does |
| --- | --- |
| `BRIDGE_STRIP` | Removes the component. The entity stays, even with nothing left on it. |
| `BRIDGE_SWEEP` | Removes the component, and destroys an entity that has no component left. |
| `BRIDGE_DELETE` | Destroys each entity that has the component, with all else that is on it. |

Storage applies the policy of the module that unloads, and only when it is the last module that declares the component.
For a sweep, an entity has nothing left when it is in no table, in no other sparse set, and has no other direct-indexed value.
A stored id of an entity that a sweep or a delete destroyed is stale, and the check of its generation refuses it.

A strip never fails, and never costs an entity.
Storage works it a table at a time, and it never needs more than what is there.
After it, there are no more tables than before, and no more bytes.
For each table that carries the component:

| The table | What storage does |
| --- | --- |
| Its entities carry nothing else. | They have nothing left, in no table, and storage releases the table. |
| No table of what is left exists. | The table becomes that table where it stands: the column goes, the columns after it close up, and no row moves. |
| A table of what is left has room for the rows. | The rows join that table, and storage releases this one. |
| A table of what is left has no room, and the column region cannot give it. | The table becomes a second table of that set, where it stands. |

The last case is the one case with a trace, and storage says it.
One record names the component, how many rows stay apart, and `[budget] columns`, which had no room to join them.
It is a sign that this budget is nearly used up.
Two tables then hold the same archetype, until one of them has no rows, and a module sees a span for each.
An entity that gets that set later goes to the table that was there first.
An entity in either table stays there while its archetype does not change.

## Functions

| Function | Result |
| --- | --- |
| `storage_drop_cells` | Drops a direct-indexed component from each entity that has a value of it. In `src/storage_drop_apart.c`. |
| `storage_drop_set` | Drops a component of a sparse set, an entity at a time, from the last. In `src/storage_drop_apart.c`. |
| `storage_drop_single` | Drops a singleton. It is on no entity, so the three policies say one thing: it has no value. In `src/storage_drop_apart.c`. |
| `storage_drop_table` | Drops a component from one table that carries it. Gives how many rows stay in a second table of their archetype. In `src/storage_drop_table.c`. |
