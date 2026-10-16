# src/config.h

This header gives the settings of a module: one value of the component that its author named, written after `init` by what each field is.
`docs/intern/config.c.md`, *The settings component*, tells what the settings are.

## The text of the settings

Bridge writes the text of the settings of a module at `init`, into the persistent share of that module.
`init` runs again at each load of the module.
Where the share of a module stays at its unload, a new copy at each load would waste one block for each cycle.
So where a share stays, Bridge remembers the block of the text in it.
The next load of the module writes there again if the text fits, and takes from its new share only if it does not.
`docs/intern/config_kept.c.md` keeps these places, and `docs/intern/config_room.c.md` finds where the text goes.

## Types

| Type | Content |
| --- | --- |
| `config_seed_t` | The settings of a module while they are written. |
| `config_seeder_fn_t` | A function that writes one setting into a field of one kind. It gives 1 if it refuses the setting. |
| `config_integer_t` | The integers that a field of an integer kind holds: `least` to `most`, and `size`, its length in bytes. |

The fields of `config_seed_t`:

| Field | Content |
| --- | --- |
| `b`, `m`, `cfg` | The world, the module, and the config of the module. |
| `c` | The component that the settings are a value of. |
| `row` | That value, while it is written. |
| `text_used` | How much of the block of text of the module the settings before this one took. |
| `why`, `why_len` | Where the reason of a refusal goes, and its length. |

## Functions

| Function | Source | Result |
| --- | --- | --- |
| `config_seed` | `src/config.c` | Writes the settings of a module into the component that its config names. Gives 0, or 1 with `err` saying which setting or name was refused. |
| `config_keep` | `src/config_kept.c` | Remembers where the settings text of an unloaded module is: the block outlives the module, and its next load writes there. |
| `config_forget` | `src/config_kept.c` | Forgets where the settings text of a module was: its share went back to the arena, with the block. |
| `config_find_kept` | `src/config_kept.c` | Gives what is remembered of the settings text of the module `stem`, or `nullptr`. |
| `config_refuse` | `src/config_say.c` | Writes why the settings are refused, as `printf` would. Gives 1. |
| `config_say_misfit` | `src/config_say.c` | Refuses a setting whose kind the field it names does not take. `field` tells what the field is. Gives 1. |
| `config_place_text` | `src/config_room.c` | Finds where the text of the settings goes, before any is written. Gives 1 if there is no room. |
| `config_seed_setting` | `src/config_field.c` | Writes one setting into the field that its key names. Gives 1 if no field has that name, or the field does not take it. |
| `config_seed_bool` | `src/config_scalar.c` | Writes a setting into a bool field. |
| `config_seed_float` | `src/config_scalar.c` | Writes a setting into a float field. |
| `config_seed_integer` | `src/config_scalar.c` | Writes a setting into an integer field. |
| `config_seed_enum` | `src/config_enum.c` | Writes a setting into an enum field. |
| `config_seed_bitmask` | `src/config_enum.c` | Writes a setting into a bitmask field. |
| `config_seed_text` | `src/config_text.c` | Writes a setting into a `bridge_text_t` field. |
| `config_seed_struct` | `src/config_text.c` | Refuses a setting for a struct field. |
| `config_seed_array` | `src/config_text.c` | Writes a setting into an inline array. |

Each seeder gives 1 if it refuses the setting.
There is one seeder for each kind of field, and one for an inline array.
