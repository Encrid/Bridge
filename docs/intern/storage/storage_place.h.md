# src/storage/storage_place.h

This header puts an entity where the fold has it: its table, its sparse sets, its direct-indexed values and the singletons.
It also marks a new husk.
The fourth step of a barrier, and of a delete at an unload, places each entity with it: `docs/intern/storage/storage_apply.h.md`, *One path for a destroy*.

## Functions

| Function | Result |
| --- | --- |
| `place_sets` | Has the entity at `index` join each sparse set that the fold put it in, and leave each that the fold took it out of. |
| `place_cells` | Gives the entity at `index` each direct-indexed component that the fold gave it, and takes each that the fold took. Such a component has its cell already, so it is never short of room. |
| `place_singles` | Gives each singleton the value that the fold gave to index 0, which is no entity, or takes its value. |
| `place_one` | Kills an entity that the fold destroyed. Otherwise sets the bit in `husks` of an entity that the fold marked `HUSKED`, and moves the entity to its table. Then it places its sets, its cells and, for index 0, the singletons. While no type is kept apart, that is one move and no more. |
