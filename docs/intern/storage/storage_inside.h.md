# src/storage/storage_inside.h

This header gives the data of storage: the declared types, the tables, the sparse sets, the direct-indexed components and what a barrier works in.
Each storage file includes it, directly or through another storage header.

## What storage is made for

A general ECS serves any program, so it must be right at each call.
Storage serves the world, which promises more, and storage uses what each promise allows:

| The world promises | So storage |
| --- | --- |
| One thread touches the rows, and only at a barrier. | Has no lock, no stage and no data local to a thread. |
| A barrier has each change in a queue before it applies the first. | Knows the final set of an entity before it moves a row of it: `docs/intern/storage/storage_apply.c.md`, *The steps of a barrier*. |
| The systems are fixed at load. | Matches a table against them once, when it makes the table. |
| Nobody iterates through storage. | Has no iterator and no cache: a module reads spans. |
| No module can rely on an order of rows or of spans. | Fills a hole with the last row, and lists the tables of a system in the order that costs least. |
| The types are a closed set with no pointer. | Moves a row as bytes, with `memcpy`: no constructor and no hook. |
| A term names one type: no pairs, no wildcards, no or-terms. | Has one answer for a table and a system: it matches or it does not. |

## A table

An archetype is a set of components.
A table keeps one archetype: the entities that carry exactly that set, and their values.
Storage finds a table by its archetype, a bit for each declared type, through a hash: `docs/intern/storage/storage_table.h.md`, *Functions*.
No graph links the tables, because nothing walks from one table to another: storage knows the final set of an entity before it places it.

The rows of a table are one block of the column region: `docs/intern/storage/storage_layout.h.md`, *The layout of a block*.
Each column begins at a multiple of 16 bytes, `COLUMN`, as the block does.

A loop of a system over a column compiles to loads of 16 bytes, and then no load lies across two cache lines.
Measured, a loop reads 204,800 rows in 5.6 to 6.4 us from such a column.
From a column 8 bytes past it, the loop takes 8.3 us.
It costs up to 15 bytes for each column of each table.

The first room of a table is the rows that storage asks of it when it makes it, and no more.
How a table grows is in `docs/intern/storage/storage_grow.h.md`, *Growth*.
When a row leaves, the last row of the table fills its place.

There is one table for each set, with one exception.
A strip at an unload can leave two tables of one set: `docs/intern/storage/storage_drop.h.md`, *What unload leaves behind*.
Then the hash finds the table that was there first, which has `named`, and the other table lives until its rows leave.

A table lives while it has rows.
Storage makes it for the first entity that is to be in it.
Storage gives back its block and its id at the barrier where its last row leaves.
The table ids are dense, from 1. The id 0 is no table: an entity with nothing on it is there.
The next table that storage makes takes a released id again.

## A sparse set

A component in a sparse set is in no archetype.
The archetype of an entity holds only the components that tables keep, `in_table`, and its table is the table of that archetype.
To give an entity such a component, or to take it, moves no row of any table.
An entity that carries only such components is in no table.

Storage keeps the set as a table of its one type, with `set` of the table at the slot of the set plus one.
Its block holds the ids of the entities in the set, then their values, packed.
It grows as the table of an archetype does, and storage gives it back when the last entity leaves the set.
There is room for a table for each sparse set beside `[budget] tables`, so storage never refuses the table of a set.

Storage finds the row of an entity in the set through its own index, `where`.
Its cell for an entity holds the row plus one, or 0 for none.
Storage writes a second index for modules in the shared memory, at the offset `sparse` of the world: `docs/extern/bridge_abi.h.md`, *Storage layouts*.
When the block of the set moves, storage writes again the cell of each entity in it.

The two indexes take 8 bytes for each entity there may be, for each sparse set.
`[budget] sparse_sets` gives how many sets there may be, and storage gets their indexes when it makes the world.

## Direct-indexed

A direct-indexed component has a cell for each entity there may be, and the value of the entity with index `e` is in cell `e`.
It is in no archetype, as a component in a sparse set is in none, and to give it or take it moves no row.
Its cells are one block of the column region: 16 bytes, then its size for each of `[budget] entities`.
Storage takes the block at `declare`, from `[budget] columns`, and a `declare` that the block does not fit fails.
After that, an entity always has its cell.

A cell exists for an entity that has no value, so a bit for each entity tells whether it has one.
An attach sets the bit and a detach clears it, at the barrier, and storage tells the observers as for any other layout.
When an entity goes, storage clears its bit and its cell, so the next entity at that index has nothing of the old one.
Storage keeps the bits in `given`, and writes a copy for modules in the shared memory, at the offset `direct` of the world.
The first 4 bytes of the 16 bytes before the cells hold the offset of that copy from the shared base.

`[budget] direct_indexed` gives how many such components there may be, and storage gets their bits when it makes the world.
The cost is the size of the component for each entity, carried or not: 64 bytes at 400,000 entities is 25.6 MB of `[budget] columns`.

## A husk

A destroy can leave an entity with the components that it keeps, and such an entity is a husk: `docs/extern/bridge_abi.h.md`, *A husk*.
`husk_types` is the set of the types whose `on_destroy` is `BRIDGE_ON_DESTROY_KEEP` and that a loaded module declares.
A husk is a live place with its index and its generation, in the table of the types that it keeps, or in none.
The entity index shows it as any other entity, so a module finds there the types that the husk keeps, and no other type.
The bit of the entity in `husks` makes it a husk, and only storage reads the bit.
`docs/intern/storage/storage_husk.h.md` tells what a fold does with a husk, and `docs/intern/storage/storage_husk.c.md` how the set changes.

The mark is not in the entity index.
Modules read that copy and can write over it, and storage never reads it: *Nothing in the shared block is read back*.
The mark is not in `place_t` either: a field there makes each place 16 bytes, in place of 12.
`husks` takes one bit for each of `[budget] entities`, and storage gets it when it makes the world.

## Nothing in the shared block is read back

Storage keeps these in memory of its own, which no module can reach:

| Data | Where |
| --- | --- |
| Where each entity is: table, row and generation | `places`, by index |
| Which entity is in which row, for each table | `who`, in step with the column region |
| Each set, and the offset of each column | `sets` and `columns`, by table |
| Which entity is a husk | `husks`, by index |

The entity index, the array of ids of each table and the table directory in the shared block are copies.
Storage writes them when it places a row, and never reads them.
A module that writes over them misleads itself and the modules that read them after, but it does not change what storage does next.
The bytes of a component are different: a column in the block is the data.
A module that writes a term it has does what the term is for.

## Constants

| Constant | Content |
| --- | --- |
| `DEAD` | `0xffffffff`. The table of a place whose entity is gone, and the generation that the entity index shows for it. |
| `HELD` | `0xffffffff`. The row of a place in table 0 whose id a block holds and `spawn` has not handed out. |
| `COLUMN` | 16. The multiple at which each column and each block begins, and the bytes before a toggled column or direct-indexed cells. |

## Types

### enum folded

What the fold decided of an entity, for the length of one barrier, in `folded`.

| Flag | Content |
| --- | --- |
| `TOUCHED` | The barrier changes the entity, and it is in `changed`. |
| `DESTROYED` | The entity is gone at this barrier. |
| `DROPPED` | Storage leaves the entity as it was, for want of a table or of columns. |
| `SHORT` | With `DROPPED`: for want of columns, and not of a table. |
| `MOVES` | The entity is to have a row that it does not have, and storage finds room for it. |
| `HUSKED` | A destroy at this barrier left the entity some types of `husk_types`, so it is a husk from this barrier on. |

### storage_type_t

A declared type, `struct storage_type`.

| Field | Content |
| --- | --- |
| `id` | Its bit in a set, and its column in the layout of a table. |
| `size` | The bytes of one value. 0 for a tag. |
| `align` | Where a column of it may begin: a power of two. |
| `kept` | Its storage layout. |
| `slot` | Which sparse set it is, or which direct-indexed component it is. |
| `cells`, `cells_len` | Direct-indexed: its block, which is 16 bytes and then a cell for each entity. A singleton: its block, which is its one value. |
| `set` | A singleton: it has a value. |

### storage_terms_t

The terms of one system or observer, `struct storage_terms`.

| Field | Content |
| --- | --- |
| `count` | How many terms it has. |
| `type`, `oper` | Room for `[budget] terms` of each, of which the first `count` are its own. |
| `list` | A system: the tables that it reads. |
| `pos` | A system: for each table, its place in `list` plus one, and 0 for a table that `list` does not hold. |
| `matched` | A system: how many tables `list` holds. |
| `changed` | Which tables it reads is not what storage last wrote for its module. |
| `used` | This record of the `[budget] systems` records is taken. |
| `events`, `heard`, `ctx` | An observer: what `storage_listener_t` gave. |

### place_t

Where an entity is: `table`, `row` and `generation`.
For a free place, `table` is `DEAD` and `row` is the next free index.

### table_t

A table.

| Field | Content |
| --- | --- |
| `rows` | How many rows it has. |
| `room` | How many rows its block has room for. |
| `block`, `block_len` | Its block of the column region, as an offset from the shared base, and its length. |
| `incoming` | The rows that this barrier adds. Storage finds room for them first. |
| `live` | The table exists. |
| `touched` | It changed since storage last wrote what modules read, and it is in `dirty`. |
| `sized` | Storage found room for this barrier: `incoming` is then how many rows it has room for. |
| `named` | The hash finds this table for its set. |
| `set` | The slot of the sparse set that this table keeps, plus one. 0 for the table of an archetype. |

### storage_t

The storage of one world, `struct storage`.
The budget sizes it when the world is made: `docs/intern/block/block_budget.c.md`, *The budget of a deployment*.

| Field | Content |
| --- | --- |
| `w` | The world. |
| `places` | Where each entity is, by index. Never the copy in the block. |
| `high` | The indexes from `high` on were never used. |
| `free_head` | The first free index. The free indexes make a list through `row` of their places. |
| `tables`, `table_room` | The tables, by id, and how many ids there may be beside 0: `[budget] tables` plus `[budget] sparse_sets`. |
| `table_free`, `table_next` | The free list of table ids. |
| `table_high` | The table ids from `table_high` on were never used. |
| `hash`, `hash_room` | A set to its table, by open addressing. `hash_room` is a power of two: `docs/intern/storage/storage_made.c.md`, *The hash of tables*. |
| `sets` | For each table, which types its entities carry: `words` words each. |
| `columns` | For each table and type, where the column of the type is, and 0 if it has none. |
| `col_hash` | For each table and type, the last hash of its column, for the checksum pass. Beside `columns`, and with its shape. |
| `words`, `type_room` | How wide a set is, in 64-bit words and in types. |
| `terms_room` | `[budget] systems`. |
| `who` | Which entity is in which row: a twin of the column region, a cell for each 8 bytes of it. |
| `dirty`, `dirty_count` | The tables whose `touched` is set: *table_t*. |
| `types` | The declared types, by id, and `nullptr` where none is declared. |
| `type_pool`, `terms_pool` | The records of the types, and of the terms of `[budget] systems` systems and observers. |
| `lists` | The `list` and `pos` of each record of terms, one after the other. |
| `term_types`, `term_opers` | The terms of each record of terms. |
| `systems`, `system_count` | The terms of each system. |
| `observers`, `observer_count` | The terms of each observer, in the order they came. |
| `work` | For each entity, the set it is becoming in the fold. |
| `changed`, `changed_count` | The entities that this barrier changes, in the order the fold met them. |
| `folded` | For each entity, its flags of `enum folded`. |
| `dest` | For each entity, the table it will be in. |
| `at` | Where `lay_out` last put the column of each type. |
| `attached` | For each entity, the types that an attach wrote a value of. Kept only while there is an observer. |
| `scratch`, `before`, `masked` | One set each, for the work of a barrier. |
| `set_room`, `set_type`, `set_table` | `[budget] sparse_sets`, and by slot the type of each set and its table, 0 while no entity is in it. |
| `where` | For each slot and entity, its row in the set plus one, and 0 for none. |
| `in_table` | The set of the types that tables keep. |
| `table_count` | How many tables of archetypes there are. |
| `tags` | How many declared types have no bytes. |
| `apart` | How many declared types no table keeps. While there is none, an entity carries what its table holds, and a barrier looks no further. |
| `direct_room`, `bit_words`, `direct_type` | `[budget] direct_indexed`, the words of a bit for each entity, and the type of each slot. |
| `given` | For each direct-indexed slot, a bit for each entity that has a value. |
| `husk_types` | The types that a destroy leaves on an entity: *A husk*. |
| `husks` | A bit for each entity, set while the entity is a husk: *A husk*. |

### row_move_t

A row on its way from one table to another: the entity at `index` leaves row `row` of `from` for row `at` of `to`.
Either table may be 0, which is no table.

### seen_t

What an observer is told of: the id of an entity, the set it carried, `before`, and the set it carries now, `after`.
`attached` is the set of what an attach wrote a value of.
`before` and `after` are null for an entity that carried nothing, or that is gone.

### value_reader_t

A function that reads the bytes of a value, `read`, and the context it gets, `ctx`.

### convert_t

The rows of components that storage writes again as other types: the storage `r`, the `moves`, their `count`, and `was`.
`was` holds a copy of the values of one entity as they were, one after the other.

## Functions

The header declares no function.
