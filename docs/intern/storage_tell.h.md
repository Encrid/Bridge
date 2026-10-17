# src/storage_tell.h

This header destroys an entity, and tells each observer what it is owed for an entity.
It compares what the entity carried with what it carries.

## What an observer is owed

An observer runs when what its terms describe changes for an entity.
It asks for some of 3 events: `BRIDGE_ON_ENTER`, `BRIDGE_ON_LEAVE` and `BRIDGE_ON_ATTACH`.
Storage tells it what the barrier made of an entity, and not each change asked of it.
That is the difference between what the entity carried before the barrier and what it carries after:

| The entity | Event |
| --- | --- |
| Its set did not match the terms before the barrier, and matches them after. | Enter. |
| Its set matched the terms before the barrier, and does not after, or the entity is gone. | Leave. |
| It matches the terms, and an attach of one of its terms that is not `BRIDGE_NOT` is still on it. | Attach, with enter if it matches only now. |

Enter and leave are about the terms of the observer, and not about what was done to the entity.
Attach is about the entry `attach`, and not about a value.
Storage does not hear of a write in place, so the write raises nothing.
An entity that carries nothing is in no table, and matches nothing.

So an observer of two terms hears an entity when it comes to carry both, by the attach that completes the pair.
An entity that gets a component and loses it in one phase is not heard.
The other cases are in `docs/extern/bridge_abi.h.md`, *observe*.
An observer gets one call for an entity at a barrier, with the events that hold.
Storage tells the observers in their order: `docs/intern/storage_terms.c.md`, *A record of terms*.

## Functions

| Function | Result |
| --- | --- |
| `is_alive` | Gives whether the entity of an id exists. Its index is not 0, storage used it, its place is not `DEAD`, and it has the generation of the id. |
| `leave_apart` | Takes the entity at `index` out of each sparse set it is in, and takes from it each direct-indexed value. |
| `kill_entity` | Makes the entity at `index` gone: its row, and its index, which comes back with the next generation. The entity index shows `DEAD`, and the generation goes back to 1 before it reaches `DEAD`. |
| `get_owed` | Gives what one observer is owed for an entity, as the table above says, limited to the events it asked for. Each caller gives the set `before` of storage, so it is never null. |
| `tell_observers` | Calls `heard` of each observer that is owed something for an entity, in the order the observers came. |
