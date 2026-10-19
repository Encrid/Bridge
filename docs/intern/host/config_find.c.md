# src/host/config_find.c

This file finds the value of a key across the files, and prints a found value.

## The search

`config_find` reads the local file first, since it overrides the rest.
`find_in` reads a file, and otherwise descends into the files that it names.
When two peers state a key whose values differ, it returns both as a clash.
`config_find_key` finds a key and reports it through `config_say_found`.
`config_say_value` prints the key, its value, its place, and any file that it overrides.

## Functions

| Function | Result |
| --- | --- |
| `config_find` | Gives the value the host takes, with a peer when they clash. |
| `config_find_key` | Finds a key by its names, or the empty result on a problem. |
| `config_say_value` | Prints one key and its value on a line. |
