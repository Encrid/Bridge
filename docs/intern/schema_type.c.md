# src/schema_type.c

This file reads the type of a field: a given type, `bridge_text_t`, or a type that its namespace declared.
The type set is `docs/intern/schema.h.md`, *The type set*.

## The order

`schema_set_type` looks for a given type first, then for `bridge_text_t`, and then asks `find` of the source.
So a field of one of those names never reaches `find`.
A given type has the alignment of its size.
`bridge_text_t` has the size and the alignment of `bridge_text_t` of `include/bridge_abi.h`.
A declared type has the size and the alignment that `find` gives.

## Functions

| Function | Result |
| --- | --- |
| `get_field_kind` | Gives what a field of a declared type holds: `SCHEMA_STRUCT` for a struct, `SCHEMA_ENUM` for an enum, and `SCHEMA_BITMASK` for any other kind. |
| `set_named_type` | Reads the token as a type that the namespace declared, through `find`. Gives false if the token is not a name, or the namespace declared nothing of that name. |
| `schema_set_type` | Reads the token as the type of a field, and writes its kind, its size and its alignment. Gives 1 if it is neither a given type nor a declared one. |
