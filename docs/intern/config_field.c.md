# src/config_field.c

This file writes a setting into the field that its key names, by what the field is.

## The field of a key

The key of a setting is the name of a field of the config component.
A key that names no field is refused: `docs/intern/config.c.md`, *The settings component*.
A field with more than one element is an inline array, and `config_seed_array` writes it.
Any other field is written by the seeder of its kind.

## The seeders

| Kind of the field | Seeder | Source |
| --- | --- | --- |
| `bool` | `config_seed_bool` | `src/config_scalar.c` |
| `char`, and the integers from `int8_t` to `uint64_t` | `config_seed_integer` | `src/config_scalar.c` |
| `float`, `double` | `config_seed_float` | `src/config_scalar.c` |
| An enum | `config_seed_enum` | `src/config_enum.c` |
| A bitmask | `config_seed_bitmask` | `src/config_enum.c` |
| `bridge_text_t` | `config_seed_text` | `src/config_text.c` |
| A struct | `config_seed_struct`, which refuses | `src/config_text.c` |

A field of a kind with no seeder is refused as a type that no setting fills.

## Functions

| Function | Result |
| --- | --- |
| `find_field` | Gives the field of the config component that a key names, or `nullptr`. |
| `seed_field` | Writes a setting into a field: an inline array by itself, and any other field by the seeder of its kind. |
| `config_seed_setting` | Writes one setting into the field that its key names. Gives 1 if no field has that name, or the field does not take it. |
