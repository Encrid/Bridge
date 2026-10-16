# src/storage_publish.c

This file writes what a module reads at a barrier.
It also forgets the touched tables after storage wrote for each module.
What storage writes, and when, is in `docs/intern/storage_publish.h.md`, *What a barrier writes for a module*.

## The span region

Storage writes into the span region of the private memory of the module, from `spans` of the module, in this order:

1. A `bridge_query_t` for each system of the module.
2. A `bridge_list_t` for each observer.
3. `column_base`: a cell for each table id and each declared type.
4. `kept`: a cell for each declared type.
5. The spans of each system, one after the other, from a multiple of 8.
6. The records of the lists of the observers.

Items 1 to 4 always fit: `check_spans` checked them at the load, and their counts do not change after it.
If the spans or the lists do not fit in `spans_len` bytes, `storage_publish` gives 1, and `src/block_barrier.c` stops the module.

## The lists of the observers

The list of an observer holds a `bridge_trigger_t` for each entity that triggered it since the last barrier: its id and its events.
Storage writes the lists when an observer has an entity, and when one had an entity at the last barrier.
Then a list with no entity gets a count of 0.
The records of the lists follow the spans, so storage also writes the lists when it wrote all the spans again.

## Functions

| Function | Result |
| --- | --- |
| `write_list` | Writes observer `q` of a module its list of the entities that triggered it since the last barrier. Gives 1 if it does not fit. |
| `storage_publish_lists` | Writes the list of each observer of a module, when the rule above asks for it. Gives 1, with the observer in `full`, if a list does not fit. |
| `storage_publish` | Writes what a module reads: its `column_base`, its spans and the lists of its observers. Gives 1 if they do not fit in its span region, and names in `list` the observer whose list did not fit. |
| `storage_end_publish` | Forgets which tables were touched, after storage wrote for each module. |
| `storage_count_tables` | Gives how many table ids there are, 0 included, which is no table: `table_room` plus 1. |
