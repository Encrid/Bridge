# src/storage_spans.c

This file writes the spans of a module: for each of its systems, one span for each table that it reads.

## A span

A span has `BRIDGE_SPAN_SIZE` bytes for its count of terms.
It holds the count of rows, where the entity ids are, and then a base for each term.
The entity ids are at the start of the block of the table.
The base of a term is what `get_handed` gives for its column.
The base is 0 for a term `BRIDGE_NOT`, which must not match, and for a component that the table does not hold.

## Where the spans go

The spans of a system are one after the other, and its `bridge_query_t` says where they begin and how many there are.
The module can write its `bridge_query_t`, so storage keeps where the spans begin in `spans_at` of the system, and reads only that.
The spans of the systems of a module are one after the other too.
So when a system reads other tables than it did, storage writes all the spans of the module again, from the start.
Otherwise it writes again, where it is, each span whose table was touched, which it finds through `pos` of the system.
A system with no record of terms has no span.

A span is a multiple of 4 bytes, and the records after the spans hold ids.
So storage rounds the end of the spans up to 8.
If the spans do not fit in the span region, storage gives 1.

## Functions

| Function | Result |
| --- | --- |
| `write_span` | Writes one span of table `t` for a system. |
| `write_spans` | Writes each span of system `q` of a module, one after the other, and where they begin. Gives 1 if they do not fit. |
| `rewrite_spans` | Writes again, where it is, each span of system `q` whose table was touched. |
| `write_every_span` | Writes all the spans of a module again, and where they end. Gives 1 if they do not fit. |
| `storage_publish_spans` | Writes the spans of a module. It writes all of them again if a system reads other tables than it did, and otherwise the touched ones. |
