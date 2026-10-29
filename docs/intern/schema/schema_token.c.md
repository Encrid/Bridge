# src/schema/schema_token.c

This file tells what a token of a descriptor is: a name, an integer, or a given type.
It also copies a text, and takes the room of the fields or of the constants.

## Integers

`schema_get_integer` reads a token that begins with a minus sign as signed, with `strtoll`.
It reads a token that begins with a digit as unsigned, with `strtoull`, and keeps its bits.
So the bits of a bitmask of 64 bits stay as they are.
Both reads use base 0: `0x10` is 16, and a number that begins with 0 is octal, so `010` is 8.
The token is an integer only if the read takes all of it, and `errno` stays 0.

A read that takes no character leaves `end` on the first character, a digit or a minus sign, which is not the zero byte.
So the test that `end` is on the zero byte also tells that the read took a character.

## Functions

| Function | Result |
| --- | --- |
| `schema_is_name` | Gives whether the token is a name that a field or a constant may have. A name begins with a letter or an underscore, and is not cut. |
| `schema_get_integer` | Reads the token as an integer, and says whether it is below zero. Gives false if it is not an integer, or if the token was cut. |
| `schema_find_given` | Gives the index in `SCHEMA_GIVEN` of the type that the token names, or -1. |
| `schema_copy_text` | Gives a copy of a text in memory from `malloc`, or `nullptr` if there is no memory. |
| `schema_take_room` | Gives zeroed room for one item of `size` bytes for each `mark` of the text, and one more. Gives `nullptr` if there is no memory. |
