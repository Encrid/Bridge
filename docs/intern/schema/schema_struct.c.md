# src/schema/schema_struct.c

This file reads a struct from its descriptor, a field at a time, and lays it out as C lays it out.
The layout rule is `docs/extern/bridge_schema.h.md`, *The type set*.

## A field

A field is a type, a name, a length in brackets for an inline array, and a semicolon.
The length is an integer from 1, and at most `UINT32_MAX` divided by the size of the type.

## Limits

The reader refuses a field whose size is more than `UINT32_MAX - offset - 8`, where `offset` is where the field begins.
No type of the set has an alignment above 8, so the 8 keeps room for the padding at the end of the struct.
The check does not hold for an offset above `UINT32_MAX - 8`: the subtraction then wraps.

Before the first field, the reader takes the room of the fields once: one field for each `;` of the text, and one more.
A field is added before its `;` is read, so the one more holds a last field with no `;`. Thus the array never grows, and no number sizes it.
If there is no memory for the array, the reader refuses the descriptor at its brace. With no memory for a copy of a name, it refuses it at that name.

The reader does not refuse two fields of one name.

## Functions

| Function | Result |
| --- | --- |
| `add_field` | Adds a field that the token names, in the room of the fields. Gives 1 when its name cannot be copied for want of memory. |
| `read_length` | Reads the length of an inline array, from its opening bracket to past its closing one. Sets `count` of the field, and multiplies its size. Gives 1 if refused. |
| `place_field` | Places the field just added at the next multiple of its alignment, and counts it in the size and the alignment of the struct. It adds in 64 bits. Gives 1 if the size cannot be counted in 32 bits. |
| `read_field` | Reads one field, from its type to its semicolon, and lays it out. Gives 1 if refused. |
| `schema_parse_struct` | Takes the room of the fields, and reads them to the closing brace. Rounds the size up to a multiple of the widest field. Refuses a struct with no field. Gives 1 if refused. |
