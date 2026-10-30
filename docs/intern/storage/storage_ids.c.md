# src/storage/storage_ids.c

This file sets ids aside for a module to hand out, and destroys the ones it did not hand out.
It also gives a singleton its value outside a barrier.

## Ids set aside

A module hands out ids from a block that storage sets aside for it, and each id names a real entity.
`storage_reserve` takes each index from the free list, or the next index never used, which starts at generation 1.
An entity set aside has nothing on it: it is in no table, and no query matches it.
Its row is `HELD` until the barrier after `spawn` hands it out: `src/storage/storage_held.c`.
It counts against `[budget] entities`.
When no index is left, `storage_reserve` takes fewer ids than asked, and gives how many it took.

The free list gives the index that was freed last first, with no search.
This order is fixed, and is no key. The Usage promises no order of the ids, and each reused index gets a new generation.

`storage_give_single` writes the value of a singleton outside a barrier, for the settings of a module.
Index 0 comes to carry the singleton, and storage tells the observers before it writes the value.
It tells them that only this singleton is attached, so an observer of another singleton hears no `BRIDGE_ON_ATTACH`.

## Functions

| Function | Result |
| --- | --- |
| `take_index` | Gives an entity index that nobody has: a freed one, or the next one never used, at its first generation. Gives 0 if no entity is left. |
| `storage_reserve` | Takes up to `want` ids with nothing on them, and writes each in `ids`. The entity index shows an id only from its hand-out. Gives how many it took. |
| `storage_release` | Destroys each id of `ids`, with what is on it. Gives back the tables of the sparse sets that have no row. Each id is alive: a held id, which no change touches, or an id of a load that fails before any barrier. |
| `storage_is_alive` | Gives whether an id names an entity that exists. |
| `storage_give_single` | Gives a singleton its value outside a barrier, and tells its observers. |
