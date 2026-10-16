# src/schema_struct.c

This file reads a struct from its descriptor, a field at a time, and lays it out as C lays it out.
The layout rule is `docs/intern/schema.h.md`, *The type set*.

## A field

A field is a type, a name, a length in brackets for an inline array, and a semicolon.
The length is an integer from 1, and at most `UINT32_MAX` divided by the size of the type.

## Limits

The reader refuses a field whose size is more than `UINT32_MAX - offset - 8`, where `offset` is where the field begins.
No type of the set has an alignment above 8, so the 8 keeps room for the padding at the end of the struct.
The check does not hold for an offset above `UINT32_MAX - 8`: the subtraction then wraps.
The array of fields starts with room for 8, and doubles when it is full.
If there is no memory for the array or for a copy of a name, the reader refuses the descriptor at that name.
The reader does not refuse two fields of one name.

## Functions

| Function | Result |
| --- | --- |
| `add_field` | Adds a field that the token names, with room made for it. Gives 1 if there is no memory. |
| `read_length` | Reads the length of an inline array, from its opening bracket to past its closing one. Sets `count` of the field, and multiplies its size. Gives 1 if refused. |
| `place_field` | Places the field just added at the next multiple of its alignment, and counts it in the size and the alignment of the struct. Gives 1 if the size cannot be counted. |
| `read_field` | Reads one field, from its type to its semicolon, and lays it out. Gives 1 if refused. |
| `schema_parse_struct` | Reads the fields of a struct to its closing brace, and rounds its size up to a multiple of its widest field. Refuses a struct with no field. Gives 1 if refused. |
