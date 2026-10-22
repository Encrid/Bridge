# include/backends/wasm/bridge_wasm_span.h

This header reads a query's spans from the query list in the private header.
A span is one matched table: its id and its row count, eight bytes in all.

## Functions

| Function | Result |
| --- | --- |
| `bridge_count_spans` | Gives how many spans a query matched. |
| `bridge_get_span` | Gives the address of span `i` of a query. |
| `bridge_span_table` | Gives the id of the table that a span names. |
| `bridge_count_rows` | Gives how many rows a span has. |
