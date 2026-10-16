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
| It did not match the terms, and now matches them. | Enter. |
| It matched the terms, and no longer matches them, or it is destroyed. | Leave. |
| It matches the terms, and an attach of one of its terms that is not `BRIDGE_NOT` is still on it. | Attach, with enter if it matches only now. |

Enter and leave are about the terms of the observer, and not about what was done to the entity.
An observer of "A and not B" hears an entity that carries A enter when B is detached, and leave when B is attached.
Attach is about the entry `attach`, and not about a value.
A system that writes a component in place raises nothing, since storage does not hear of a write.
An entity that carries nothing is in no table, and matches nothing.

So an observer of two terms hears an entity when it comes to carry both, by the attach that completes the pair.
An entity made and destroyed in one phase is not heard at all, nor one that gets a component and loses it in one phase.
An entity that loses a component and gets it back in the same phase is heard as an attach, since it carries a new value.
An observer gets one call for an entity at a barrier, with the events that hold.
A detach of what is not there, and any change of a dead id, is nothing.
Storage tells the observers in the order they came.

## Functions

| Function | Result |
| --- | --- |
| `is_alive` | Gives whether an id names an entity that exists. Its index is not 0, storage used it, its place is not `DEAD`, and it has the generation of the id. |
| `leave_apart` | Takes the entity at `index` out of each sparse set it is in, and takes from it each direct-indexed value. |
| `kill_entity` | Makes the entity at `index` gone: its row, and its index, which comes back with the next generation. The entity index shows `DEAD`, and the generation goes back to 1 before it reaches `DEAD`. |
| `get_owed` | Gives what one observer is owed for an entity, as the table above says, limited to the events it asked for. |
| `tell_observers` | Calls `heard` of each observer that is owed something for an entity, in the order the observers came. |
