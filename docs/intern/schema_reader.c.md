# src/schema_reader.c

This file reads a descriptor a token at a time, and writes why a descriptor is refused.

## Tokens

A token is a run of name characters, or one other character.
A name character is a letter, a digit or an underscore, so `0x10` is one run.
A minus sign begins a run when a name character follows it, so a number below zero is one token.
The reader skips spaces, tabs, line feeds and carriage returns between tokens.
At the end of the text, the token is empty.

`word` keeps at most 63 characters and a terminating zero.
The reader skips the rest of a longer run, and sets `cut`: `schema_is_name` gives false for such a token.

## Functions

| Function | Result |
| --- | --- |
| `is_name_char` | Gives whether a character is a letter, a digit or an underscore. |
| `read_run` | Copies a run of name characters into `word`, and cuts a run that is too long. Gives how many characters it kept. |
| `schema_read_next` | Reads the next token into `word`, and clears `cut` first. |
| `schema_refuse` | Writes `what` into `why` of the source with `snprintf`. The token takes the place of `%s`, or "the end of it" at the end of the text. Gives 1. |
| `schema_is` | Gives whether the token is the text that the caller gives, by `strcmp`. |
