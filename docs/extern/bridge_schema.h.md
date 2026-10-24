# include/bridge_schema.h

`include/bridge_schema.h` gives the macros that make a type of a module in C, and its descriptor, from one text.
The descriptor is the text that Bridge reads to lay the type out as the compiler of the module does.

## The macros

The macros make a type and its descriptor of one text, so that they cannot differ.
Name the text one time, with a macro of your own, and give it to the two macros of its kind:

```c
#define POSITION_FIELDS { float x; float y; }
BRIDGE_SCHEMA_STRUCT_TYPE(position_t, POSITION_FIELDS);
BRIDGE_SCHEMA_STRUCT_DESC(position_t, POSITION_FIELDS);
```

| Macro | Result |
| --- | --- |
| `BRIDGE_SCHEMA_TEXT` | The text of its arguments, as one string. |
| `BRIDGE_SCHEMA_STRUCT_TYPE` | The struct `name`, with the body that its other arguments give. |
| `BRIDGE_SCHEMA_STRUCT_DESC` | The descriptor of the struct, `name_desc`: the text of the body. |
| `BRIDGE_SCHEMA_ENUM_TYPE` | The enum `name`, of the integer `type`, with the body that its other arguments give. |
| `BRIDGE_SCHEMA_ENUM_DESC` | The descriptor of the enum, `name_desc`: the integer, a space, and the text of the body. |
| `BRIDGE_SCHEMA_BITMASK_TYPE` | The bitmask `name`, an enum of the integer `type`, with the body that its other arguments give. |
| `BRIDGE_SCHEMA_BITMASK_DESC` | The descriptor of the bitmask, `name_desc`: the integer, a space, and the text of the body. |

A macro holds one statement only: `docs/contributing/coding_style.md`, *H3 - The macros*.
No declaration makes both a type and its descriptor, thus a pair is two macros.
`name_desc` is a `static const char` array: the name of the type, and `_desc` after it, for example `position_t_desc`.
Give it to `declare` as `desc`, with `sizeof(name_desc) - 1` as `desc_len`.
`docs/extern/bridge_abi.h.md`, *declare* tells the other fields of a declaration.
A module that compiles with `include/bridge_schema.h` is a work based on Bridge: `readme.md`, *License*.

Write the schema header of a module with these macros.
A module that shares a component includes the schema header of the module that declares it.
The descriptor is a text that Bridge reads, thus the macros are the same for a module on each backend.
A module in another language does not use this header: it writes the same text in its own way.

## The type set

A descriptor can say only this:

```text
a struct     { field field ... }
a field      type name ;    or    type name [ count ] ;
a type       bool char int8_t int16_t int32_t int64_t uint8_t uint16_t uint32_t
             uint64_t float double bridge_text_t, or a type that its namespace declared
an enum      integer { name , name = value , ... }
a bitmask    integer { name = value , ... }
```

- The integer of an enum or a bitmask is the width of the field in a row. Its author states it.
- An enum counts its constants from 0: a constant with no value is the last value plus 1.
- The integer of an enum is `int8_t`, `uint8_t`, `int16_t`, `uint16_t`, `int32_t` or `uint32_t`.
- The integer of a bitmask is `uint8_t`, `uint16_t`, `uint32_t` or `uint64_t`. A bitmask states the value of each constant.
- Each value fits in the integer of its type.
- A value is a number in decimal, in hexadecimal after `0x`, or in octal after `0`. A value below zero starts with `-`.
- A name starts with a letter or `_`, and holds letters, digits and `_`: 63 characters at most.
- An array has one length, and the length is a number.

The descriptor is the text after the preprocessor replaces each macro in it.
Thus a macro can give the length of an array, and the descriptor holds its number.
The code can get the length from the field: `sizeof` of the field divided by `sizeof` of one item.
A pointer, `size_t` and `intptr_t` are not in the set. They have 4 bytes in a module of wasm32, and 8 bytes in a native module.
Each type of the set has one size and one alignment on each target.

Bridge lays a struct out as C does. Each field is at the next multiple of its alignment.
The size of the struct is a multiple of its largest alignment.
`declare` refuses a type whose size or alignment is not what its descriptor lays out.

A field of a declared type names it with no namespace. Bridge looks for it in the namespace of the type that holds the field.
For example, `Vec3 at;` in `game:Body` is `game:Vec3`, which the module declares first.
A tag cannot be the type of a field.

For a struct of another namespace in a component, declare a struct of your own namespace from the same text. It costs one more of `types`.
`bridge_text_t` is a type of Bridge, and no module declares it: `docs/extern/bridge_abi.h.md`, *alloc*.

## Layouts

Two modules that declare one type state the same descriptor and the same kind, or `declare` refuses the second one.
Bridge compares the two descriptors byte for byte.
The `#` of the preprocessor makes each run of white space one space, and removes the comments.
Thus a new layout of the text or a new comment does not change the descriptor.
A field added, removed, of another type, or in another order changes it.

The descriptor of a type covers its own text, and not the texts of the types that its fields name.
Each of those types has its descriptor, which `declare` checks when the module declares it.
Thus `declare` refuses a module built for an older `Vec3` at `Vec3`, before `Body`.

A record names a layout by its fingerprint: the 64-bit FNV-1a hash of the descriptor, XOR the kind.
The fingerprint only names a layout, and two different descriptors can have one fingerprint.
Then the record of the refusal shows the same fingerprint two times.

## Messages

`declare` writes this reason when it refuses a descriptor, and the record of the host gives it: `docs/extern/bridge_abi.h.md`, *Messages*.
Angle brackets mark a part that changes: `docs/extern/bridge.h.md`, *Messages*.

- `type '<type>': its descriptor <reason>`

The reason gives the word at which the reading stopped, or `the end of it` where the text ended. It is one of these:

- `begins with '<word>' and not with a brace`
- `goes on after its closing brace, with '<word>'`
- `begins with '<word>' and not with the integer it is`
- `begins with '<word>', and a bitmask is an unsigned integer`
- `says '<word>' where its brace belongs`
- `says '<word>' where a constant's name belongs`
- `says '<word>' where a constant ends`
- `says '<word>' where a bit's value belongs: a bitmask states every one`
- `gives a constant the value '<word>', which the integer it is cannot hold`
- `counts up to a constant before '<word>' that the integer it is cannot hold`
- `has no constant`
- `says '<word>', which is neither a type a field can be nor one its namespace has declared`
- `says '<word>' where a field's name belongs`
- `gives '<word>' as the length of an array`
- `says '<word>' where an array's length ends`
- `says '<word>' where a field ends`
- `is more bytes than can be counted, at '<word>'`
- `has no field`
- `ran out of memory at '<word>'`

`declare` writes this reason when the declaration and the descriptor do not agree:

- `type '<type>' is <bytes> bytes aligned to <number> in the module and <bytes> aligned to <number> by its descriptor`
