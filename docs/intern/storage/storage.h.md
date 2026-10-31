# src/storage/storage.h

This header gives what Bridge asks of storage, the part of the library that keeps the rows of every component.
Storage is internal: no module, host or backend can reach it, and the files `src/storage_*` are its one implementation.

## What is asked of it

Each function has the name of a request, as the functions of the backend seam do, and not the name of a method of storage:

- At a load, a module declares a type, and hands over the terms of its systems and observers.
- At a barrier, Bridge applies the queues and writes what each module reads.
- At an unload, Bridge drops a component that nobody declares any more.

The header does not tell how storage works a barrier: `docs/intern/storage/storage_apply.c.md`, *The steps of a barrier*, tells it.
To storage, a type is a size, an alignment, a storage layout and whether a destroy keeps it, and nothing more.
`src/schema/schema.c` reads the fields of a type and the constants of an enum from its text, and Bridge keeps them outside storage.

## Storage layouts

The author of a component states its storage layout at `declare`: `docs/extern/bridge_abi.h.md`, *Storage layouts*.
Each of the 5 storage layouts has the name of what keeps the component.
These sections tell how storage keeps each layout, and a tag in each of them:

- `docs/intern/storage/storage_inside.h.md`, *A table*
- `docs/intern/storage/storage_inside.h.md`, *A sparse set*
- `docs/intern/storage/storage_bytes.h.md`, *A toggle*
- `docs/intern/storage/storage_inside.h.md`, *Direct-indexed*
- `docs/intern/storage/storage_change.h.md`, *A singleton*
- `docs/intern/storage/storage_view.h.md`, *A tag*

## Types

### bridge_world_t, module_t

The world and a loaded module. `src/inside.h` defines them.

### storage_t, storage_type_t, storage_terms_t

The storage of one world, a declared type, and the terms of one system or observer.
To the rest of the library they are opaque: `docs/intern/storage/storage_inside.h.md`, *Types*, gives their fields.

### storage_term_t

A term, as storage gets it.
Storage gets the type and the operator, and not the access: read or write is for the scheduler, and storage never uses it.

| Field | Content |
| --- | --- |
| `type` | The declared type of the term. |
| `oper` | `BRIDGE_AND`, `BRIDGE_NOT` or `BRIDGE_OPTIONAL`. |

### storage_heard_t

The function that storage calls for an observer: one call for each change of an entity, in the order storage applies the changes.
It gets the context of the observer, the id of the entity and the events that the observer is owed.

### storage_listener_t

Who gets what an observer is owed.

| Field | Content |
| --- | --- |
| `events` | The events that the observer asked for. |
| `heard` | The function that storage calls. |
| `ctx` | What storage gives to `heard` as its context. |

### storage_run_t

Bytes `from` of a value as it was, which become bytes `to` of the value as it will be.

| Field | Content |
| --- | --- |
| `from` | The offset in the old value. |
| `to` | The offset in the new value. |
| `len` | How many bytes. |

### storage_move_t

Each value of type `from` becomes a value of type `to`: zero, but for the runs that storage copies across.

| Field | Content |
| --- | --- |
| `from` | The type as it is. |
| `to` | The type as it will be. |
| `runs`, `run_count` | The runs of bytes to copy. |

### storage_usage_t

How much of two storage limits the world uses, and the most it has used.

| Field | Content |
| --- | --- |
| `entities`, `entities_peak` | The live entities, and the highest the entity index ever reached. |
| `tables`, `tables_peak` | The live tables, and the most tables ever made at one time. |

### storage_read_fn_t, storage_visit_fn_t, storage_changed_fn_t

A function that storage calls with the bytes of a value.
A function that storage calls for the index of an entity.
A function that storage calls with the type of a column that changed.

### enum storage_moved

How `storage_convert` ended.

| Value | Content |
| --- | --- |
| `STORAGE_MOVED` | Storage moved the rows. |
| `STORAGE_NO_ROOM` | Storage changed nothing: there is no room where the rows were to go. |
| `STORAGE_NO_MEMORY` | Storage changed nothing: it has no memory to hold the values while they move. |

## Constants

| Constant | Value |
| --- | --- |
| `STORAGE_HASH_MOST` | 2147483648, 2 to the power 31: the most places of the hash of tables, since `hash_room` is a power of two in 32 bits. |

## Functions

| Function | Result |
| --- | --- |
| `storage_create` | Makes the storage of a world: `docs/intern/storage/storage_create.c.md`, *Functions*. |
| `storage_destroy` | Frees the storage of a world: `docs/intern/storage/storage_create.c.md`, *Functions*. |
| `storage_declare` | Declares a type: `docs/intern/storage/storage_declare.c.md`, *Functions*. |
| `storage_count_free_sets` | Counts the free slots of sparse sets: `docs/intern/storage/storage_slots.c.md`, *Functions*. |
| `storage_count_free_direct` | Counts the free direct-indexed slots: `docs/intern/storage/storage_slots.c.md`, *Functions*. |
| `storage_undeclare` | Forgets a type: `docs/intern/storage/storage_slots.c.md`, *Functions*. |
| `storage_add_system` | Takes the terms of a system: `docs/intern/storage/storage_terms.c.md`, *Functions*. |
| `storage_add_observer` | Takes the terms of an observer: `docs/intern/storage/storage_terms.c.md`, *Functions*. |
| `storage_forget` | Forgets a record of terms: `docs/intern/storage/storage_terms.c.md`, *Functions*. |
| `storage_apply` | Applies the queue of each module: `docs/intern/storage/storage_apply.c.md`, *The steps of a barrier*. |
| `storage_validate_scan` | Reports to the caller every type with a changed column: `docs/intern/storage/storage_validate.c.md`, *Functions*. |
| `storage_validate_baseline` | Takes the columns as the baseline of the next check: `docs/intern/storage/storage_validate.c.md`, *Functions*. |
| `storage_make_writable` | Makes the set `writable` of a module: `docs/intern/storage/storage_slots.c.md`, *Functions*. |
| `storage_let_write` | Records what a module may write: `docs/intern/storage/storage_slots.c.md`, *Functions*. |
| `storage_let_keep` | Records whether a destroy keeps a type: `docs/intern/storage/storage_husk.c.md`, *Functions*. |
| `storage_reserve` | Sets ids aside for a module: `docs/intern/storage/storage_ids.c.md`, *Functions*. |
| `storage_publish` | Writes what a module reads: `docs/intern/storage/storage_publish.c.md`, *Functions*. |
| `storage_end_publish` | Forgets which tables changed: `docs/intern/storage/storage_publish.c.md`, *Functions*. |
| `storage_count_tables` | Counts the table ids: `docs/intern/storage/storage_publish.c.md`, *Functions*. |
| `storage_usage` | Tells the live and the peak entities and tables: `docs/intern/storage/storage_usage.c.md`, *Functions*. |
| `storage_is_dirty` | Tells if a table is dirty: `docs/intern/storage/storage_usage.c.md`, *Functions*. |
| `storage_count_carriers` | Counts the entities that carry a type: `docs/intern/storage/storage_carriers.c.md`, *Functions*. |
| `storage_count_off` | Counts the toggled rows that are off: `docs/intern/storage/storage_values.c.md`, *Functions*. |
| `storage_visit_carriers` | Visits each entity that carries a type: `docs/intern/storage/storage_carriers.c.md`, *Functions*. |
| `storage_convert` | Moves components to their new types: `docs/intern/storage/storage_convert.c.md`, *Functions*. |
| `storage_drop` | Drops a component that nobody declares any more: `docs/intern/storage/storage_drop.c.md`, *Functions*. |
| `storage_release` | Destroys the ids not handed out: `docs/intern/storage/storage_ids.c.md`, *Functions*. |
| `storage_hand_out` | Ends the hold on a list of ids: `docs/intern/storage/storage_held.c.md`, *The ids that a block holds*. |
| `storage_give_single` | Gives a singleton its value: `docs/intern/storage/storage_ids.c.md`, *Functions*. |
| `storage_is_alive` | Tells whether an entity exists: `docs/intern/storage/storage_ids.c.md`, *Functions*. |
| `storage_visit_values` | Reads each value of a type: `docs/intern/storage/storage_values.c.md`, *Functions*. |
