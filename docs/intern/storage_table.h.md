# src/storage_table.h

This header finds a table by its set, matches the set of a table against terms, and marks a table touched.

## What a system walks

A system walks either tables or one sparse set: `docs/extern/bridge_abi.h.md`, *bridge_term_t*.
`src/abi_term.c` refuses any other query at `add_system`: `docs/intern/abi_term.c.md`, *What a query may not ask for*.

With a term `BRIDGE_OPTIONAL`, a module reaches a component outside the tables from the row: `docs/extern/bridge_abi.h.md`, *bridge_term_t*.

A query whose one term that selects is of a sparse set walks the set, with one span: `docs/extern/bridge_abi.h.md`, *Spans*.
Its count, ids and base are those of the set, so the same loop as for a table reads it.
The other terms of the query are optional, and the module reaches them by id.

`terms_read` holds this rule: a system reads the table of a sparse set only if it has a `BRIDGE_AND` of the type of the set.
That table holds the type of the set alone, so after `terms_match` each `BRIDGE_AND` names that type, and `terms_read` looks only for a `BRIDGE_AND`.

## Functions

| Function | Result |
| --- | --- |
| `touch` | Sets `touched` of table `t`, and adds the table to `dirty` if `touched` was not set: `docs/intern/storage_inside.h.md`, *table_t*. |
| `find_table` | Gives the table that the hash finds for a set, or 0. It looks from `hash_set` on, a place at a time. |
| `terms_match` | Gives whether a set carries each type that the terms must have, and none that they must not have. An optional term does not count. |
| `terms_read` | Gives whether the spans of a system include table `t`: the set matches, and a sparse set is walked only as described above. |
| `find_set_slot` | Gives which sparse set has a type in a set, or `set_room` if none has. |
