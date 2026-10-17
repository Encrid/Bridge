# src/schema.h

This header gives a type from its text: what a descriptor may say, and what it lays out to.
It reads text and gives a layout: who keeps the rows is not its concern, and storage never sees a field.

## The descriptor

A descriptor is what `BRIDGE_SCHEMA_TEXT` of `include/bridge_schema.h` makes of the body of a type.
That is the stringification of the preprocessor: the tokens of the body, with one space between them and no comment.
The descriptor of an enum or of a bitmask begins with the integer type that its author states.
Both the compiler of the module and `schema_parse` lay out the type from the same text.
`src/abi_layout.c` refuses a type whose two layouts differ in size or in alignment.

## The type set

A descriptor may say this, and nothing else:

```text
a struct      { field field ... }
a field       type name ;   or   type name [ count ] ;
a type        bool char int8_t int16_t int32_t int64_t uint8_t
              uint16_t uint32_t uint64_t float double bridge_text_t,
              or a type its namespace declared
an enum       integer { name , name = integer , ... }
              counted from zero, or from the last
a bitmask     unsigned { name = integer , ... }
              every bit stated
```

`docs/intern/schema_reader.h.md`, *Constants*, gives the size of each given type.
`bridge_text_t` is given and not declared: an offset and a length into the persistent arena, as `include/bridge_abi.h` defines it.

A field of a type that its namespace declared names the type bare: `Vec3 at;` in `game:Body` is `game:Vec3`.
The `find` function of the source looks for the name in the namespace of the type that the descriptor describes.
The namespace declared the named type before: a struct, an enum or a bitmask.

An inline array has one length, written as a number.
The descriptor is the text before the preprocessor replaces a name, so `char title[N]` reaches Bridge as `N`, and the reader refuses it.

`schema_parse` lays out a struct as C does on every target of a module: `docs/extern/bridge_schema.h.md`, *The type set*.
This is sound because no type of the set is a pointer, holds a pointer, or has the size of a pointer.
So each type has one size and one alignment on every target.

An enum and a bitmask are the integer that their author states before the brace.
An enum is one of `int8_t`, `uint8_t`, `int16_t`, `uint16_t`, `int32_t` and `uint32_t`.
A bitmask is an unsigned integer: `uint8_t`, `uint16_t`, `uint32_t` or `uint64_t`.
An enum constant with no value is one more than the last, and the first is 0.
A bitmask states the value of every constant.
The reader checks that every constant fits the integer.

The reader also refuses a struct with no field, and an enum or a bitmask with no constant.
It refuses a struct whose size 32 bits cannot count.
A refusal quotes the token where the reading stopped, or says "the end of it".

## What it gives

`schema_parse` gives the size of the type and its alignment.
For a struct, it also gives the fields in their order.
For an enum or a bitmask, it gives the integer that the author states, and the constants in their order.
After a refusal, it gives 1, writes the reason into `why`, and leaves nothing allocated.
The reason is the rest of a sentence that begins "its descriptor", which `src/abi_layout.c` writes.

The fields stay with Bridge, and no field name reaches a module.
`src/config_field.c` uses them to write the settings of a module: `docs/intern/config_field.c.md`.
`src/abi_relay_fields.c` matches the fields of a type by name at a reload.

## Types

### enum schema_kind

What one field holds, in a `uint8_t`. The first value is 1, so 0 is none of them.

| Value | Content |
| --- | --- |
| `SCHEMA_BOOL` | `bool`. |
| `SCHEMA_CHAR` | `char`. |
| `SCHEMA_I8`, `SCHEMA_U8` | `int8_t` and `uint8_t`. |
| `SCHEMA_I16`, `SCHEMA_U16` | `int16_t` and `uint16_t`. |
| `SCHEMA_I32`, `SCHEMA_U32` | `int32_t` and `uint32_t`. |
| `SCHEMA_I64`, `SCHEMA_U64` | `int64_t` and `uint64_t`. |
| `SCHEMA_F32`, `SCHEMA_F64` | `float` and `double`. |
| `SCHEMA_ENUM` | An enum that the namespace declared. |
| `SCHEMA_BITMASK` | A bitmask that the namespace declared. |
| `SCHEMA_TEXT` | `bridge_text_t`. |
| `SCHEMA_STRUCT` | A struct that the namespace declared. |

### schema_field_t

A field of a struct.

| Field | Content |
| --- | --- |
| `name` | The name of the field: a copy that `schema_free` frees. |
| `offset` | Where the field begins in the struct, in bytes. |
| `count` | 1, or the length of an inline array. |
| `kind` | What the field holds. |
| `type` | The declared type of an enum, a bitmask or a struct field: what `find` gave for it. |

### schema_named_t

A type declared before this one, in the same namespace, as a field may name it.

| Field | Content |
| --- | --- |
| `kind` | `BRIDGE_STRUCT`, `BRIDGE_ENUM` or `BRIDGE_BITMASK`. |
| `size` | Its size in bytes. |
| `align` | Its alignment. |
| `type` | The record of the type, which goes into the `type` of a field. |

### schema_find_t

The function that finds a declared type: `bool (*)(void *ctx, const char *name, schema_named_t *named)`.
It gives false if the namespace declared nothing of that name, and otherwise writes `named`.

### schema_constant_t

One constant of an enum or of a bitmask.

| Field | Content |
| --- | --- |
| `name` | The name of the constant: a copy that `schema_free` frees. |
| `value` | Its value. A bitmask of 64 bits can have a constant above `INT64_MAX`: its bits stay as they are. |

### schema_t

A type, laid out.

| Field | Content |
| --- | --- |
| `size` | The size of the type in bytes. |
| `align` | The alignment of the type. |
| `base` | The integer that an enum or a bitmask is, as its author states it. |
| `fields`, `field_count` | The fields of a struct, in the order of its text. |
| `constants`, `constant_count` | The constants of an enum or a bitmask, in the order of its text. |

### schema_source_t

What `schema_parse` reads.

| Field | Content |
| --- | --- |
| `kind` | The kind of type that the text describes: `BRIDGE_STRUCT`, `BRIDGE_ENUM` or `BRIDGE_BITMASK`. |
| `text` | The descriptor. |
| `find`, `ctx` | How a name that a field uses is found, and what `find` gets with it. |
| `why`, `why_len` | Where a refusal is written, and its room in bytes. |

## Functions

| Function | Result |
| --- | --- |
| `schema_parse` | Lays out a type: `docs/intern/schema.c.md`, *Functions*. It is `[[nodiscard]]`. |
| `schema_free` | Frees a parsed type: `docs/intern/schema.c.md`, *Functions*. |
