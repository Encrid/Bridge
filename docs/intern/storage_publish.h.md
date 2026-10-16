# src/storage_publish.h

This header gives what storage writes for a module at a barrier: its `column_base`, its spans and the lists of its observers.

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

Storage writes all of it again for a module the first time.
It writes all of it again for each module when `layout_moved` of the deck is set: a module came or went, or a declaration changed.

## Types

### publish_t

One module that storage writes for.

| Field | Content |
| --- | --- |
| `r`, `m` | The storage, and the module. |
| `cell` | Its `column_base`: for each table and each type it declared, where the column of the type is in that table. |
| `kept` | After `column_base`, a cell for each type it declared: where it reaches a component that no table keeps. |
| `at`, `end` | Where the next thing goes in its private memory, and where its span region ends. |
| `all` | A declaration changed, or storage never wrote for the module: storage writes each table, and not only the touched ones. |
| `again` | Storage writes all the spans of the module again, since one of its systems reads other tables than it did. |

## Functions

| Function | Result |
| --- | --- |
| `storage_publish_cells` | Writes the `column_base` of a module, and its `kept` cells. In `src/storage_cells.c`. |
| `storage_publish_spans` | Writes the spans of a module: all of them, or the ones whose table was touched. Gives 1 if they do not fit. In `src/storage_spans.c`. |
| `storage_publish_lists` | Writes the list of each observer of a module, when one of them has something or had something. Gives 1 if they do not fit. In `src/storage_publish.c`. |
