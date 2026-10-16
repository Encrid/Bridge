# src/abi_make.c

This file makes a declared type.
It has everything that the type needs before it keeps any of it: `docs/intern/say.h.md`, *Out of memory*.

## The steps

1. Read and lay out the descriptor: `docs/intern/abi_layout.c.md`.
2. Have the record of the type, its name, its zero value, and room for the types its fields name.
3. Have a place for the type among the types of the deck.
4. Ask storage to declare the type, with its size, its alignment and its storage layout.
5. If a piece is missing, free what was had, and refuse the type.
6. If not, keep the type among the types of the deck, with what the declaration stated.

The zero value is `size` zero bytes: what an attach with `BRIDGE_ZEROED` copies.
A tag has no bytes, so its zero value is one byte that nothing reads.
The types that the fields name are no more than the fields, so the room for them is one a field.

Storage refuses a type for one cause only: `[budget] columns` has no room for its bytes.
These bytes are the value of a singleton, or the cells of a direct-indexed type.
The refusal then gives the size of the type and the bytes of `[budget] columns`.

## A type kept

If the type replaces a type at a reload, the old type is marked `superseded`, and the new type keeps it in `replaces`.
The new type holds each declared type that one of its fields names.
It counts as a user of each such type, for as long as it is there.

## Functions

| Function | Result |
| --- | --- |
| `free_type` | Frees a declared type that is not kept, with its name, its zero value and its list of types. |
| `alloc_type` | A new declared type with its name, its zero value, and room for the types its fields name. Also its place among the types of the deck. |
| `say_unmade` | Frees what was had of a type that is not made, and refuses it: for want of memory, or of room in `[budget] columns`. Gives 1. |
| `keep_type` | Keeps a made type among the types of the deck, and holds each declared type that a field of it names. |
| `abi_make_type` | Makes the type that a declaration states. 1 with a refusal. |

`alloc_type` gives nullptr, with nothing kept, if there is not the memory for all of it.
