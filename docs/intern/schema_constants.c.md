# src/schema_constants.c

This file reads an enum or a bitmask from its descriptor: the integer it is, then its constants.
What the integer may be, and how a constant gets its value, is `docs/intern/schema.h.md`, *The type set*.

## The reading

The first token names the integer, and the size and the alignment of the type are those of that integer.
A brace follows, then the constants, then the closing brace.
A constant is a name, then its value, then a comma or the closing brace.
A comma may follow the last constant.

The array of constants starts with room for 8, and doubles when it is full.
If there is no memory for the array or for a copy of a name, the reader refuses the descriptor at that name.
The reader does not refuse two constants of one name.

## Functions

| Function | Result |
| --- | --- |
| `set_base` | Reads the token as the integer of the type, and sets `base`, `size` and `align` of `out`. Refuses an integer that the kind does not allow, and a signed integer for a bitmask. |
| `add_constant` | Adds a constant that the token names, with room made for it. Gives 1 if there is no memory. |
| `read_constant` | Reads one constant, from its name to the comma or the brace after it. Gives 1 if refused. |
| `schema_parse_constants` | Reads the integer, the brace and the constants of an enum or of a bitmask. Refuses a text with no constant. Gives 1 if refused. |
