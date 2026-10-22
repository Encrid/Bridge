# src/host/toml/toml_number.c

This file reads a whole number or a real number.

`copy_digits` copies the span of number characters and drops an underscore that lies between two digits.
It rejects an underscore that is not between digits.
`toml_read_number` turns down a leading zero, a following colon, and a following lowercase letter.
A number with a dot or an `e` goes to `strtod` as a real; any other goes to `strtoll` as a whole number.
The reader refuses a value that overflows or that leaves a character behind.

## Functions

| Function | Result |
| --- | --- |
| `toml_read_number` | Reads a whole or real value, or refuses a number that is not taken. |
