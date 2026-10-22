# src/lib/storage/storage_publish.h

This header gives what storage writes for a module at a barrier.

## What a barrier writes for a module

A module reads 4 things that storage writes into its private memory:

- its spans,
- its `column_base`,
- the lists of its observers,
- where it reaches each type that no table keeps.

Storage writes only what changed.
It keeps a list, `dirty`, of the tables touched since it last wrote: grown, moved, made or released, or with a row more or less.
For each module, storage writes this:

| What | When storage writes it |
| --- | --- |
| `column_base` | The cells of each touched table. |
| A span | Again where it is, if its table was touched. |
| The spans of the queries | All again, when the tables that one query of the module reads changed: the spans of the queries are one after the other. |
| The list of an observer | When it has something, or had something at the last barrier. |

A barrier at which nothing changed writes nothing, however many tables there are.
A span exists for each table that has rows, since a table with no row does not exist.

Storage writes all of it for a new record, since `src/lib/bridge/bridge_load.c` and `src/lib/bridge/bridge_reload.c` set `layout_moved` before its first barrier.
It writes all of it again for each module when `layout_moved` of the world is set: a module came or went, or a declaration changed.

## Types

### publish_t

One module that storage writes for.

| Field | Content |
| --- | --- |
| `r`, `m` | The storage, and the module. |
| `cell` | Its `column_base`: `docs/extern/lib/bridge_abi.h.md`, *Spans*. |
| `kept` | After `column_base`, a cell for each type it declared: where it reaches a component that no table keeps. |
| `rows`, `blocks` | Per table: the row count, and where the ids are in the shared memory. |
| `stride`, `write`, `kind` | Per type: the size in bytes, the write flag, and the storage kind. |
| `owned` | The owned ranges of the module in the shared arena: a count, then the ranges. |
| `at`, `end` | Where the next thing goes in its private memory, and where its span region ends. |
| `all` | A declaration changed, or storage never wrote for the module: storage writes each table, and not only the touched ones. |
| `again` | Storage writes every span of the module again: `docs/intern/lib/storage/storage_spans.c.md`, *Where the spans go*. |
| `full` | The name of the observer whose list did not fit, or null. |

`lay_private` places these arrays in the private memory, one after another, and leaves `at` past them for the spans.

## Functions

| Function | Result |
| --- | --- |
| `storage_publish_cells` | Writes `column_base` and `kept`: `docs/intern/lib/storage/storage_cells.c.md`, *Functions*. |
| `storage_publish_spans` | Writes the spans of a module: `docs/intern/lib/storage/storage_spans.c.md`, *Functions*. |
| `storage_publish_lists` | Writes the lists of the observers: `docs/intern/lib/storage/storage_publish.c.md`, *Functions*. |
| `lay_private` | Lays the arrays of the capability table in the private memory, and sets `at` past them. |
