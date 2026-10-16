# src/config_scalar.c

This file writes a setting into a field that is a bool, a float or an integer.

## A bool

A bool takes a bool setting: one byte, 1 or 0.
Any other setting is refused as a misfit.

## A float

A float field of 32 or 64 bits takes a float setting or an integer setting.
A field of 32 bits gets the value made narrower.
Any other setting is refused as a misfit.

## An integer

An integer field takes an integer setting that it can hold.
The value goes into the field as its low bytes, as many as the field has, because each target is little-endian.
An integer that the field cannot hold is refused, and any other setting is refused as a misfit.

| Field | Least | Most | Bytes |
| --- | --- | --- | --- |
| `char`, `int8_t` | `INT8_MIN` | `INT8_MAX` | 1 |
| `uint8_t` | 0 | `UINT8_MAX` | 1 |
| `int16_t` | `INT16_MIN` | `INT16_MAX` | 2 |
| `uint16_t` | 0 | `UINT16_MAX` | 2 |
| `int32_t` | `INT32_MIN` | `INT32_MAX` | 4 |
| `uint32_t` | 0 | `UINT32_MAX` | 4 |
| `int64_t` | `INT64_MIN` | `INT64_MAX` | 8 |
| `uint64_t` | 0 | `INT64_MAX` | 8 |

A setting is an `int64_t`, so a `uint64_t` field gets at most `INT64_MAX`.

## Functions

| Function | Result |
| --- | --- |
| `config_seed_bool` | Writes a bool into a bool: one byte, 1 or 0. Gives 1 if it refuses. |
| `config_seed_float` | Writes a float or an integer into a float of 32 or 64 bits. Gives 1 if it refuses. |
| `put_integer` | Writes an integer into an integer field that holds it. Gives 1 if it does not fit. |
| `config_seed_integer` | Writes an integer into an integer field, if the field holds it. Gives 1 if it refuses. |
