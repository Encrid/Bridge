# src/host/toml/toml_add.c

This file appends a finished statement to a `toml_t` and guards its key against a clash.

## The clash

`whole_key` joins the name of the current table with the key of the statement.
`get_clash` weighs the joined key against every earlier entry.
It rejects a key that is stated twice, and a key that is both a value and a table.
`toml_add` grows the array of entries and stores the key, its depth, the start line, and the value.

## Functions

| Function | Result |
| --- | --- |
| `toml_has_prefix` | Tests whether an entry begins with a list of names. |
| `toml_add` | Stores an entry, or refuses on a clash or on no memory. |
