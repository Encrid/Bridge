# src/host/toml_say.c

This file compares two values and renders a value as text.

`toml_is_same` holds when two values share a kind and equal contents; for a list it walks the items in turn.
`say_scalar` prints a text in quotes, a whole number with `%lld`, a real with `%g`, and a flag as a word.
`toml_say` prints a list inside brackets with the items parted by a comma, and recurses for each item.

## Functions

| Function | Result |
| --- | --- |
| `toml_is_same` | Gives true when two values are equal. |
| `toml_say` | Writes a value into a buffer of a given size. |
