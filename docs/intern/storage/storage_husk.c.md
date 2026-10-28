# src/storage/storage_husk.c

This file keeps the set of the types that a destroy leaves on a husk.
It also deletes the carriers of a component at an unload, as a destroy does.

## The types that a husk keeps

`husk_types` is in `docs/intern/storage/storage_inside.h.md`, *A husk*.
`storage_let_keep` puts a type in it or takes the type out, with `set_write`.
Each new slot of a module calls it, with true for a type of `BRIDGE_ON_DESTROY_KEEP`: `docs/intern/abi/abi_declare.c.md`, *The slots*.
Before an unload drops a type, `module_release_types` takes out each type that only the leaving module declares: `docs/intern/module/module_types.c.md`, *The steps*.

`storage_declare` does not get `on_destroy`.
The set changes when a declarer comes or goes, and not when a type is made.

## A delete at an unload

A delete destroys each carrier of the component as a destroy does: `docs/extern/bridge_abi.h.md`, *bridge_on_unload*.
`storage_delete_carriers` takes the steps of a barrier, with no queue and no value:

1. It folds each carrier with `fold_carrier`, through `storage_visit_carriers`.
2. `fold_carrier` starts the fold of the entity, takes the component out, and calls `fold_husk`.
3. `reserve_all` finds a table and room for each husk. A husk without room stays as it was.
4. `tell_all` tells the observers, and `place_all` places each entity that does not stay as it was.
5. `forget_all` forgets the fold.
6. It gives how many entities still carry the component: those that had no room for a husk.

`storage_drop` then takes the component from them as a strip does, and writes a record: `docs/intern/storage/storage_drop.c.md`, *By layout*.
The fold is over before that strip, since `is_swept` reads `HUSKED` through `is_husk`.
No stake applies: the author of the component stated the delete at `declare`.
A carrier that is a husk keeps its other types of `husk_types`, or goes when none is left.

## Functions

| Function | Result |
| --- | --- |
| `storage_let_keep` | Puts a type in `husk_types` when `keep` is true. Else it takes the type out of the set. |
| `fold_carrier` | Folds the delete of the carrier at `index`, as `storage_visit_carriers` calls it. The context is the deleted type. |
| `storage_delete_carriers` | Destroys each carrier of a type as a destroy does, through the steps of a barrier. Gives how many carriers are left for want of room for their husks. |
