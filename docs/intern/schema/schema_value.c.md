# src/schema/schema_value.c

This file gives the value of a constant, and whether the integer of its type can hold it.
How a constant gets its value is `docs/intern/schema/schema.h.md`, *The type set*.

## Fitting

A value below zero fits a signed integer if it is not below the minimum of that integer.
It fits no unsigned integer.
A value that is not below zero is compared as its bits with the maximum of the integer.
So `uint64_t` holds every value that is not below zero.

The base is one of seven integers, since `set_base` refuses each other kind: `docs/intern/schema/schema_constants.c.md`.
`schema_fits` has a case for six of them, and `uint64_t`, the base of a bitmask of 64 bits, is its default.

## Functions

| Function | Result |
| --- | --- |
| `schema_fits` | Gives whether the integer `base` can hold a value. A base that is none of the six integers of its cases is `uint64_t`. |
| `read_value` | Reads the value after an equals sign, and the token after it. Refuses a value that is not an integer, or that the integer of the type cannot hold. |
| `schema_set_value` | Gives the constant just named its value: the one after an equals sign, or one more than the last. Refuses a bitmask constant with no value, and a counted value that the integer cannot hold. |
