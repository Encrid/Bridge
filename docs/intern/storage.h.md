# src/storage.h

This header gives what Bridge asks of storage, the part of the library that keeps the rows of every component.
Storage is internal: no module, host or backend can reach it, and the files `src/storage_*` are its one implementation.

## What is asked of it

Each function has the name of a request, as the functions of the backend seam do, and not the name of a method of storage:

- At a load, a module declares a type, and hands over the terms of its systems and observers.
- At a barrier, Bridge applies the queues and writes what each module reads.
- At an unload, Bridge drops a component that nobody declares any more.

The header does not tell how storage works a barrier: `docs/intern/storage_apply.c.md`, *The steps of a barrier*, tells it.
To storage, a type is a size, an alignment and a storage layout, and nothing more.
`src/schema.c` reads the fields of a type and the constants of an enum from its text, and Bridge keeps them outside storage.

## Storage layouts

The author of a component states its storage layout at `declare`, because the code of a module that reads the component depends on it.
There are 5 storage layouts, and each has the name of what keeps the component:

| Layout | What keeps the component | What it is for |
| --- | --- | --- |
| `BRIDGE_TABLE` | The table of the archetype of its entity: a column beside the other components of the entity. | What systems read together, row by row. |
| `BRIDGE_SPARSE_SET` | Its own packed array, and an index by entity. | What comes and goes, and what few entities carry. |
| `BRIDGE_TOGGLE` | A column of a table, and a bit for each row that tells whether the component is on. | What an entity always has and sometimes does not use. |
| `BRIDGE_DIRECT_INDEXED` | A cell for each entity: the value of entity `i` is in cell `i`. | What a module reaches by id much more often than it walks it. |
| `BRIDGE_SINGLETON` | One value for the deployment, on no entity. | A value of which there is one, such as the settings of a module. |

A tag is a component with no bytes, and each of the 5 layouts can keep it: `docs/intern/storage_view.h.md`, *A tag*.
These sections tell how storage keeps each layout:

- `docs/intern/storage_inside.h.md`, *A table*
- `docs/intern/storage_inside.h.md`, *A sparse set*
- `docs/intern/storage_bytes.h.md`, *A toggle*
- `docs/intern/storage_inside.h.md`, *Direct-indexed*
- `docs/intern/storage_change.h.md`, *A singleton*

## Types

### bridge_t, module_t

The world and a loaded module. `src/inside.h` defines them.

### storage_t, storage_type_t, storage_terms_t

The storage of one world, a declared type, and the terms of one system or observer.
To the rest of the library they are opaque: `docs/intern/storage_inside.h.md`, *Types*, gives their fields.

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

### storage_read_fn_t, storage_visit_fn_t

A function that storage calls with the bytes of a value.
A function that storage calls for the index of an entity.

### enum storage_moved

How `storage_convert` ended.

| Value | Content |
| --- | --- |
| `STORAGE_MOVED` | Storage moved the rows. |
| `STORAGE_NO_ROOM` | Storage changed nothing: there is no room where the rows were to go. |
| `STORAGE_NO_MEMORY` | Storage changed nothing: there is no memory to carry the values across. |

## Functions

| Function | Result |
| --- | --- |
| `storage_create` | Makes the storage of a world, sized by its budget. Gives `nullptr` and writes a record if it cannot. In `src/storage_create.c`. |
| `storage_destroy` | Frees the storage of a world. In `src/storage_create.c`. |
| `storage_declare` | Declares a type of `size` bytes, kept in the layout `kept`. Gives `nullptr` only when `[budget] columns` has no room for its cells or its value. In `src/storage_declare.c`. |
| `storage_count_free_sets` | Gives how many more types a sparse set can keep. In `src/storage_slots.c`. |
| `storage_count_free_direct` | Gives how many more types can be direct-indexed. In `src/storage_slots.c`. |
| `storage_undeclare` | Forgets a type that nobody declares any more. In `src/storage_slots.c`. |
| `storage_add_system` | Takes the terms of a system. In `src/storage_terms.c`. |
| `storage_add_observer` | Takes the terms of an observer, and who gets what it is owed. In `src/storage_terms.c`. |
| `storage_forget` | Forgets the terms of a system or of an observer. In `src/storage_terms.c`. |
| `storage_apply` | Applies the queue of each module, in the order of `docs/intern/storage_runs.h.md`, *The order of the queues*. In `src/storage_apply.c`. |
| `storage_make_writable` | Makes the set of the types that a module may write. Gives false if there is no memory. In `src/storage_slots.c`. |
| `storage_let_write` | Records that a module may write a type, which gives it a stake in an entity that carries the type. In `src/storage_slots.c`. |
| `storage_reserve` | Takes up to `want` ids with nothing on them, for a module to hand out. Gives how many it took. In `src/storage_ids.c`. |
| `storage_publish` | Writes what a module reads: its spans, its `column_base` and the lists of its observers. Gives 1 if they do not fit, with the observer whose list did not fit. In `src/storage_publish.c`. |
| `storage_end_publish` | Forgets which tables changed, after storage wrote for each module. In `src/storage_publish.c`. |
| `storage_count_tables` | Gives how many table ids there are, with 0, which is no table. In `src/storage_publish.c`. |
| `storage_count_carriers` | Gives how many entities carry a type. In `src/storage_carriers.c`. |
| `storage_count_off` | Gives how many of the entities that carry a toggled type have it off. In `src/storage_values.c`. |
| `storage_visit_carriers` | Calls `visit` for each entity that carries a type, in each layout. In `src/storage_carriers.c`. |
| `storage_convert` | Writes the rows of components again as other types, all of them or none. Gives an `enum storage_moved`. In `src/storage_convert.c`. |
| `storage_drop` | Drops the rows of a component that nobody declares any more, by the policy of its last declarer. In `src/storage_drop.c`. |
| `storage_release` | Destroys ids that storage set aside and no module handed out, with what is on them. In `src/storage_ids.c`. |
| `storage_hand_out` | Ends the hold on ids that a module handed out, so that a destroy of them can pass. In `src/storage_held.c`. |
| `storage_give_single` | Gives a singleton its value outside a barrier, and tells its observers. In `src/storage_ids.c`. |
| `storage_is_alive` | Gives whether an id names an entity that exists. In `src/storage_ids.c`. |
| `storage_visit_values` | Gives `read` the bytes of each value of a type. In `src/storage_values.c`. |
