# src/backends/wasm/sdk/bridge_wasm_span.h

This header reads a query's spans from the query list in the private header.
A span is one matched table: its row count, its entity column, and a base for each term.

## Functions

| Function | Result |
| --- | --- |
| `bridge_count_spans` | Gives how many spans a query matched. |
| `bridge_get_span` | Gives the address of span `i` of a query, for a term count. |
| `bridge_count_rows` | Gives how many rows a span has. |
| `bridge_get_entities` | Gives a span's entity column. |
| `bridge_get_base` | Gives a span's base for a term. |
