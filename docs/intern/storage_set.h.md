# src/storage_set.h

This header gives a set of types: a bit for each type, in 64-bit words.

## A set of types

A set has a word for each 64 types there may be, which is one word in most deployments.
Type `t` is bit `t & 63` of word `t >> 6`.
`set_copy` and `set_clear` work a word at a time.
A `memcpy` of a set is a call, since the compiler cannot see its length: `docs/intern/storage_bytes.h.md`, *Copies of values*.
Storage makes a set for each entity that a barrier changes.

## Functions

| Function | Result |
| --- | --- |
| `set_has` | Gives whether a set holds a type. |
| `set_put` | Puts a type in a set. |
| `set_take` | Takes a type out of a set. |
| `set_copy` | Copies a set, a word at a time. |
| `set_clear` | Makes a set empty, a word at a time. |
