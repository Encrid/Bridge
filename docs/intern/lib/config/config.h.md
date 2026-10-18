# src/lib/config/config.h

This header gives the settings of a module: one value of the component that its author named, written after `init` by what each field is.
`docs/intern/lib/config/config.c.md`, *The settings component*, tells what the settings are.

## The text of the settings

Bridge writes the settings text of a module at `init`, into the persistent share of that module.
`init` runs again at each load of the module.
Where the share of a module stays at its unload, a new copy at each load would waste one block for each cycle.
So where a share stays, Bridge remembers the block of the text in it.
The next load of the module writes there again if the text fits, and takes from its new share only if it does not.
`docs/intern/lib/config/config_kept.c.md` keeps these places, and `docs/intern/lib/config/config_room.c.md` finds where the text goes.

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
| `writing` | False while the settings are checked, and true while they are written. |
| `why`, `why_len` | Where the reason of a refusal goes, and its length. |

## Functions

| Function | Source | Result |
| --- | --- | --- |
| `config_seed` | `src/lib/config/config.c` | Checks or writes the settings: `docs/intern/lib/config/config.c.md`, *Functions*. `err` says which setting or name was refused. |
| `config_keep` | `src/lib/config/config_kept.c` | Keeps a share that stays, for the next load of its module: `docs/intern/lib/config/config_kept.c.md`, *The next load*. |
| `config_give_back_free` | `src/lib/config/config_kept.c` | Gives back the free shares: `docs/intern/lib/config/config_kept.c.md`, *Functions*. |
| `config_find_kept` | `src/lib/config/config_kept.c` | Finds a kept share: `docs/intern/lib/config/config_kept.c.md`, *Functions*. |
| `config_refuse` | `src/lib/config/config_say.c` | Says why the settings are refused: `docs/intern/lib/config/config_say.c.md`, *Functions*. |
| `config_say_misfit` | `src/lib/config/config_say.c` | Refuses a setting of the wrong kind: `docs/intern/lib/config/config_say.c.md`, *Functions*. |
| `config_check_settings` | `src/lib/config/config_field.c` | Refuses a wrong setting: `docs/intern/lib/config/config_field.c.md`, *Functions*. |
| `config_place_text` | `src/lib/config/config_room.c` | Finds the room of the settings text: `docs/intern/lib/config/config_room.c.md`, *Functions*. |
| `config_seed_setting` | `src/lib/config/config_field.c` | Writes one setting: `docs/intern/lib/config/config_field.c.md`, *Functions*. |
| `config_seed_bool` | `src/lib/config/config_scalar.c` | Writes a bool field: `docs/intern/lib/config/config_scalar.c.md`, *Functions*. |
| `config_seed_float` | `src/lib/config/config_scalar.c` | Writes a float field: `docs/intern/lib/config/config_scalar.c.md`, *Functions*. |
| `config_seed_integer` | `src/lib/config/config_scalar.c` | Writes an integer field: `docs/intern/lib/config/config_scalar.c.md`, *Functions*. |
| `config_seed_enum` | `src/lib/config/config_enum.c` | Writes an enum field: `docs/intern/lib/config/config_enum.c.md`, *Functions*. |
| `config_seed_bitmask` | `src/lib/config/config_enum.c` | Writes a bitmask field: `docs/intern/lib/config/config_enum.c.md`, *Functions*. |
| `config_seed_text` | `src/lib/config/config_text.c` | Writes a `bridge_text_t` field: `docs/intern/lib/config/config_text.c.md`, *Functions*. |
| `config_seed_struct` | `src/lib/config/config_text.c` | Refuses a struct field: `docs/intern/lib/config/config_text.c.md`, *Functions*. |
| `config_seed_array` | `src/lib/config/config_text.c` | Writes an inline array: `docs/intern/lib/config/config_text.c.md`, *Functions*. |

Each seeder gives 1 if it refuses the setting.
There is one seeder for each kind of field, and one for an inline array.
