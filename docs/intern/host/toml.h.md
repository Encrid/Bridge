# src/host/toml.h

This header defines the values and statements of the TOML reader and declares its functions.
A `value_t` is tagged by `value_kind` and holds a flag, a whole number, a real number, a text, or a list.
An `entry_t` is one statement: a key of several names, a line, and a value.
A `toml_t` owns the entries, and keeps the first reason with its line when the reader refuses.
A `reading_t` is the cursor: the text position, the current table, and the output.

## Functions

| Function | Result |
| --- | --- |
| `toml_read` | Reads a whole text into a `toml_t`. |
| `toml_refuse`, `toml_skip` | Keep the first reason, and step over spaces and tabs. |
| `toml_read_pair`, `toml_read_inline`, `toml_read_key`, `toml_read_value`, `toml_read_text`, `toml_read_number` | Read one part of the grammar. |
| `toml_add`, `toml_has_prefix` | Add a statement, and test a key prefix. |
| `toml_is_same`, `toml_say` | Compare two values, and print one. |
