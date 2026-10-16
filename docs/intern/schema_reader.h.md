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
| `schema_read_next` | Reads the next token into `word`. `src/schema_reader.c` defines it. |
| `schema_refuse` | Writes why the descriptor is refused, with the token where the reading stopped. Gives 1. `src/schema_reader.c` defines it. |
| `schema_is` | Gives whether the token is a text. `src/schema_reader.c` defines it. |
| `schema_is_name` | Gives whether the token is a name that a field or a constant may have: it begins with a letter or an underscore. `src/schema_token.c` defines it. |
| `schema_get_integer` | Reads the token as an integer: below zero, or as its bits, which can be above `INT64_MAX`. Gives false if it is not one. `src/schema_token.c` defines it. |
| `schema_find_given` | Gives which of `SCHEMA_GIVEN` the token names, or -1. `src/schema_token.c` defines it. |
| `schema_copy_text` | Gives a copy of a text that `free` gives back, or `nullptr`. `src/schema_token.c` defines it. |
| `schema_fits` | Gives whether the integer `base` can hold a value. `src/schema_value.c` defines it. |
| `schema_set_value` | Gives the constant just named its value: the one stated after an equals sign, or one more than the last. Gives 1 if refused. `src/schema_value.c` defines it. |
| `schema_set_type` | Reads the token as the type of a field: what it is, and what it takes in a row. Gives 1 if refused. `src/schema_type.c` defines it. |
| `schema_parse_struct` | Reads the fields of a struct to its closing brace, and lays them out. Gives 1 if refused. `src/schema_struct.c` defines it. |
| `schema_parse_constants` | Reads the integer and the constants of an enum or a bitmask, to its closing brace. Gives 1 if refused. `src/schema_constants.c` defines it. |
