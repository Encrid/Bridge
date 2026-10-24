# src/storage/storage_carried.h

This header gives everything an entity carries: what its table holds, each sparse set it is in, and each direct-indexed value it has.

## What an entity carries

An entity carries what its table holds.
While no type is kept apart from tables, `apart` is 0, and `get_carried` looks no further.
Otherwise the entity also carries the type of each sparse set that it is in.
It also carries each direct-indexed component that it has a value of.
Index 0 is no entity: it carries each singleton that has a value.

## Functions

| Function | Result |
| --- | --- |
| `add_sets` | Adds to a set the type of each sparse set that the entity at `index` is in. |
| `add_cells` | Adds to a set the type of each direct-indexed component that the entity at `index` has a value of. |
| `add_singles` | Adds to a set each singleton that has a value. |
| `get_carried` | Writes into a set everything that the entity at `index` carries. |
