# src/storage_table.h

This header finds a table by its set, matches the set of a table against terms, and marks a table touched.

## What a system walks

A system walks tables, or one sparse set alone.
The rows of a table are in the order of the table, and the entries of a set are in the order of the set.
So row 0 of one and entry 0 of the other are different entities, and one loop cannot step through both.
So the terms that select, `BRIDGE_AND` and `BRIDGE_NOT`, are all of components that tables keep.
Or there is one term that selects, a `BRIDGE_AND` of a component in a sparse set.
`src/abi_term.c` refuses any other query at `add_system`.

A module reaches a component that no table keeps from the row, through a term `BRIDGE_OPTIONAL`.
In a query that walks tables, the term of a component in a sparse set has a base of 0 in each span.
The component is in no table.
The module reads the index of the set, which it finds through its cell for each type it declared.
For row `r`, take `e`, the low 32 bits of `entities[r]`.
Cell `e` of the index holds the offset of the value, or 0 if the entity is not in the set.

A query whose one term that selects is of a sparse set walks the set.
It has one span: its rows are the entities in the set, its `entities` are their ids, and its base is their values.
These are the three things that the span of a table holds, so the same loop reads them.
The other terms of the query are optional, and the module reaches them by id.

`terms_read` holds this rule: a system reads the table of a sparse set only if it has a `BRIDGE_AND` of the type of the set.
That table holds the type of the set alone, so after `terms_match` each `BRIDGE_AND` names that type, and `terms_read` looks only for a `BRIDGE_AND`.

## Functions

| Function | Result |
| --- | --- |
| `touch` | Marks a table touched since storage last wrote what modules read, and adds it to `dirty` once. |
| `find_table` | Gives the table that the hash finds for a set, or 0. It looks from `hash_set` on, a place at a time. |
| `terms_match` | Gives whether a set carries each type that the terms must have, and none that they must not have. An optional term does not count. |
| `terms_read` | Gives whether the spans of a system include table `t`: the set matches, and a sparse set is walked only as described above. |
| `find_set_slot` | Gives which sparse set has a type in a set, or `set_room` if none has. A set that holds the type of a sparse set is the table of that set. |
