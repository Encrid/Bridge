# src/storage/storage_spans.c

This file writes the spans of a module: for each of its systems, one span for each table that it reads.

## A span

A span is 8 bytes, the value of `BRIDGE_SPAN_SIZE`, whatever the count of terms.
It holds the id of the table, and then the count of rows of the table.
A span carries no column now: a module finds a column through the capability table that `src/storage/storage_cells.c` writes.
The id of a row lives in the block of the table, which `blocks` of the header points to.

## Where the spans go

The spans of a system are one after the other, and its `bridge_query_t` says where they begin and how many there are.
The module can write its `bridge_query_t`, so storage keeps where the spans begin in `spans_at` of the system, and reads only that.
The spans of the systems of a module are one after the other too.
So when a system reads other tables than it did, storage writes all the spans of the module again, from the start.
Otherwise it writes again, where it is, each span whose table was touched, which it finds through `pos` of the system.
A system with no record of terms has no span.

Each span is 8 bytes, and the spans begin on an 8-byte bound.
So storage rounds the end of the spans up to 8.
If the spans do not fit in the span region, storage gives 1.

## Functions

| Function | Result |
| --- | --- |
| `write_span` | Writes the span of table `t`: its id and its row count. |
| `write_spans` | Writes each span of system `q` of a module, one after the other, and where they begin. Gives 1 if they do not fit. |
| `rewrite_spans` | Writes again, where it is, each span of system `q` whose table was touched. |
| `write_every_span` | Writes all the spans of a module again, and where they end. Gives 1 if they do not fit. |
| `storage_publish_spans` | Writes the spans of a module. It writes all of them again if a system reads other tables than it did, and otherwise the touched ones. Gives 1 if they do not fit. |
