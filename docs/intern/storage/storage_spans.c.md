# src/storage/storage_spans.c

This file writes the spans of a module: for each of its systems, one span for each table that it reads.

## A span

A span is 4 bytes, the value of `BRIDGE_SPAN_SIZE`, whatever the count of terms: the id of a table.
A span carries no column and no row count.
A module finds a column through the capability table that `src/storage/storage_cells.c` writes.
It finds the row count and the block of a table in the table directory of the shared memory: `docs/intern/block/block_layout.c.md`.

## Where the spans go

The spans of a system are one after the other, and its `bridge_query_t` says where they begin and how many there are.
They are a copy of `list` of the record of terms of the system, which holds the id of each table that it reads.
The spans of the systems of a module are one after the other too.
So when a system reads other tables than it did, storage writes all the spans of the module again, from the start.
A change of rows changes no span, since a span holds no row count.
A system with no record of terms has no span.

The spans begin on an 8-byte bound, and storage rounds the end of the spans up to 8.
If the spans do not fit in the span region, storage gives 1.

## Functions

| Function | Result |
| --- | --- |
| `write_spans` | Copies the table ids of system `q` of a module, one after the other, and writes where they begin. Gives 1 if they do not fit. |
| `write_every_span` | Writes the spans of each system of a module, and rounds their end up to 8. Gives 1 if they do not fit. |
| `storage_publish_spans` | Writes the spans of a module again when a system reads other tables, or when the layout moved. Gives 1 if they do not fit. |
