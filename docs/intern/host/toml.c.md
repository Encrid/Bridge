# src/host/toml.c

This file drives the TOML reader: it reads a file line by line into a `toml_t`.

## The loop

`toml_read` skips a leading byte-order mark, then handles each line until the text ends.
A line that opens with `[` is a table header; another line with content is a pair.
`read_header` reads the table name, rejects an array of tables, and keeps the table for the pairs below it.
`end_line` steps past a trailing comment and insists that nothing else sits after a statement.
`toml_refuse` records the first reason with its line. `toml_skip` moves across spaces and tabs.

## Functions

| Function | Result |
| --- | --- |
| `toml_read` | Reads the whole text and gives true, else false with a reason kept. |
| `toml_refuse` | Keeps the first reason and line, and gives false. |
| `toml_skip` | Advances over the spaces and tabs at the cursor. |
