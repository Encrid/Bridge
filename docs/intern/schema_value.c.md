# src/schema_value.c

This file gives the value of a constant, and whether the integer of an enum or a bitmask can hold it.
How a constant gets its value is `docs/intern/schema.h.md`, *The type set*.

## Fitting

A value below zero fits a signed integer if it is not below the minimum of that integer.
It fits no unsigned integer.
A value that is not below zero is compared as its bits with the maximum of the integer.
So `uint64_t` holds every value that is not below zero.

## Functions

| Function | Result |
| --- | --- |
| `schema_fits` | Gives whether the integer `base` can hold a value. Gives false for a base that is not an integer that an enum or a bitmask can be. |
| `read_value` | Reads the value after an equals sign, and the token after it. Refuses a value that is not an integer, or that the integer of the type cannot hold. |
| `schema_set_value` | Gives the constant just named its value: the one after an equals sign, or one more than the last. Refuses a bitmask constant with no value, and a counted value that the integer cannot hold. |
