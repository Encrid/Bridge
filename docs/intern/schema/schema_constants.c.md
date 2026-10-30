# src/schema/schema_constants.c

This file reads an enum or a bitmask from its descriptor: the integer it is, then its constants.
What the integer may be, and how a constant gets its value, is `docs/intern/schema/schema.h.md`, *The type set*.

## The reading

The first token names the integer, and the size and the alignment of the type are those of that integer.
A brace follows, then the constants, then the closing brace.
A constant is a name, then its value, then a comma or the closing brace.
A comma may follow the last constant.

The reader takes the room of the constants once, as it takes the room of the fields: `docs/intern/schema/schema_struct.c.md`, *Limits*.
It counts each `,` of the text, and one more, since each constant before the last ends with a `,`.
The reader does not refuse two constants of one name.

## Functions

| Function | Result |
| --- | --- |
| `set_base` | Reads the token as the integer of the type, and sets `base`, `size` and `align` of `out`. Refuses an integer that the kind does not allow, and a signed integer for a bitmask. |
| `add_constant` | Adds a constant, named by the token, in the room of the constants. Gives 1 if no memory is left for a copy of its name. |
| `read_constant` | Reads one constant, from its name to the comma or the brace after it. Gives 1 if refused. |
| `schema_parse_constants` | Reads the integer and the brace, takes the room of the constants, and reads the constants of an enum or of a bitmask. Refuses a text with no constant. Gives 1 if refused. |
