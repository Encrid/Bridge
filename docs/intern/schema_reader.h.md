# src/schema_reader.h

This header gives a descriptor that is being read: the token the reader is at, and what the files that read a descriptor share.
The text that the reader reads is `docs/intern/schema.h.md`, *The descriptor*.
What it may say, and how a type is laid out, is `docs/intern/schema.h.md`, *The type set*.

## Constants

| Constant | Content |
| --- | --- |
| `SCHEMA_GIVEN_COUNT` | 12: how many types a field can be without its namespace declaring them. |
| `SCHEMA_GIVEN` | Those 12 types, as a `static const` array of `schema_given_t`. |

The alignment of a given type is its size.

| Name | Kind | Size |
| --- | --- | --- |
| `bool` | `SCHEMA_BOOL` | 1 |
| `char` | `SCHEMA_CHAR` | 1 |
| `int8_t` | `SCHEMA_I8` | 1 |
| `uint8_t` | `SCHEMA_U8` | 1 |
| `int16_t` | `SCHEMA_I16` | 2 |
| `uint16_t` | `SCHEMA_U16` | 2 |
| `int32_t` | `SCHEMA_I32` | 4 |
| `uint32_t` | `SCHEMA_U32` | 4 |
| `int64_t` | `SCHEMA_I64` | 8 |
| `uint64_t` | `SCHEMA_U64` | 8 |
| `float` | `SCHEMA_F32` | 4 |
| `double` | `SCHEMA_F64` | 8 |

## Types

### schema_given_t

A type that a field can be without its namespace declaring it.

| Field | Content |
| --- | --- |
| `name` | Its name in a descriptor. |
| `kind` | What a field of it holds. |
| `size` | Its size in bytes, which is also its alignment. |

### schema_reader_t

The reading of one descriptor.

| Field | Content |
| --- | --- |
| `at` | The next character to read. |
| `word` | The token just read: a name, a number or one mark, in 64 bytes. |
| `cut` | The token was longer than a name may be. |
| `source` | What `schema_parse` reads. |
| `out` | The type being laid out. |
| `room` | How many fields or constants `out` has memory for. |
| `value`, `below` | The value of the last constant, and whether it is below zero: the next constant counts up from it. |

### schema_laid_t

A field with what it takes in a row, before the reader places it.

| Field | Content |
| --- | --- |
| `field` | The field. |
| `size` | Its size in bytes: the size of its type, times its length for an array. |
| `align` | Its alignment. |

## Functions

| Function | Result |
| --- | --- |
| `schema_read_next` | Reads the next token: `docs/intern/schema_reader.c.md`, *Functions*. |
| `schema_refuse` | Writes the reason of a refusal: `docs/intern/schema_reader.c.md`, *Functions*. |
| `schema_is` | Compares the token with a text: `docs/intern/schema_reader.c.md`, *Functions*. |
| `schema_is_name` | Whether the token is a name: `docs/intern/schema_token.c.md`, *Functions*. |
| `schema_get_integer` | Reads the token as an integer: `docs/intern/schema_token.c.md`, *Functions*. |
| `schema_find_given` | Finds a given type: `docs/intern/schema_token.c.md`, *Functions*. |
| `schema_copy_text` | Copies a text: `docs/intern/schema_token.c.md`, *Functions*. |
| `schema_fits` | Whether an integer can hold a value: `docs/intern/schema_value.c.md`, *Functions*. |
| `schema_set_value` | Gives a constant its value: `docs/intern/schema_value.c.md`, *Functions*. Gives 1 if refused. |
| `schema_set_type` | Reads the type of a field: `docs/intern/schema_type.c.md`, *Functions*. |
| `schema_parse_struct` | Reads a struct: `docs/intern/schema_struct.c.md`, *Functions*. |
| `schema_parse_constants` | Reads an enum or a bitmask: `docs/intern/schema_constants.c.md`, *Functions*. |
