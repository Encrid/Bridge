# src/module_types.c

This file stops the declarations of a module: the components that it declared leave with it.

## The policies at unload

An unload drops the components that only the module declared, and keeps the components that other modules declare too.
For each component, its author states at `declare` what becomes of the entities that carry it, with `on_unload`:

| Policy | What happens |
| --- | --- |
| `strip` | The component is removed. The entity stays, even with nothing left on it. |
| `sweep` | The component is removed. An entity left with no component is destroyed. |
| `delete` | Each entity that has the component is destroyed, with all that is on it. |

No single answer fits, so the author states it.
An entity has no owner, because several modules put components on one, so it cannot die with the module that spawned it.
Its id sits as a plain `u64` in memories that Bridge cannot trace, so it cannot be collected when nothing reaches it.
But the author knows what the component is: an attribute of something that exists anyway, or the reason that the entity exists.

`sweep` exists because no module can list every entity: a query needs terms.
An entity with no component and no id that anybody keeps could never be destroyed, and it would count against `[budget] entities` for ever.
`delete` takes the components of other modules on those entities with it, and its author states that at `declare`.

The policy that applies is that of the module that unloads, and only when it is the last to declare the component.
Two declarers need not agree, because the first to leave drops nothing.
A stored id of an entity that `sweep` or `delete` destroyed is refused as stale, by its generation.
So an entity that must outlive the unload of another module carries a component of the module that holds its id.
How storage drops the rows is in `src/storage_drop.c`.

## The steps

`module_release_types` releases the types of a module from the last declared to the first, because a type is used by what was declared after it.
For each type:

1. Lower the count of its declarers by one. If others still declare it, stop.
2. Drop its rows by the policy of the module, if a policy applies.
3. If no field of another type names it, delete its record.
4. Mark that the layout of the deck moved.

No policy applies after a load that failed: nothing can carry a component that was never applied.
A type that a field of another component still names stays as a type, with no rows.
Deleting a record takes the type out of storage and out of the list of the deck.
It also lets go of the types that its fields name, and each of those goes too when nothing else declares or names it.

## Functions

| Function | Result |
| --- | --- |
| `unlist_type` | Takes a type out of the list of the deck: the last type takes its place. |
| `delete_type` | Deletes the record of a component, with what storage knows of it and its hold on the types that its fields name. |
| `release_type` | Stops one declaration of a type. The last declarer applies its policy, and a policy of zero is a load that failed. |
| `module_release_types` | Stops the declarations of a module, last declared first. With `by_policy` false, no policy applies. |
