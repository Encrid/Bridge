# src/lib/abi/abi_declare.h

This header gives what the files that declare a type share.
These files check the type, make it, and lay it out again at a reload.

## Types

| Type | Content |
| --- | --- |
| `abi_others_t` | The other loaded modules that declare a type, in `text`, written as the end of a refusal. |
| `declaring_t` | A type being declared. |
| `relaying_t` | A struct being laid out again at a reload. |
| `relay_at_t` | Where in a value a struct is being matched. |

`abi_others_t` names the modules because a host needs to know them: to unload them is what lets the layout of a component change.

### declaring_t

| Field | Content |
| --- | --- |
| `m` | The module that declares. |
| `d` | What the module stated. |
| `desc` | The descriptor, in the scratch arena. |
| `text` | The copy of the descriptor that the parse made, until the type keeps it. |
| `name` | The name, copied. |
| `bare` | Where the namespace ends in `name`: the colon. |
| `print` | The fingerprint of the descriptor and of the kind. |
| `c` | The type that has that name already, and then the type declared. |
| `was` | At a reload, the type of that name that this one replaces. |
| `s` | The layout, read from the descriptor. |

### relaying_t

| Field | Content |
| --- | --- |
| `m` | The module that reloads. |
| `rows` | How many rows hold a value of the type that changes. |
| `runs`, `run_count` | The runs of bytes carried over. |
| `lost`, `lost_len` | By name, what goes and this reload does not accept. |
| `short_of` | There was no memory for a run. |

### relay_at_t

| Field | Content |
| --- | --- |
| `was` | The fields of the struct as it was. |
| `is` | The fields of what the struct becomes. |
| `from` | Where the struct is in a value as it was. |
| `to` | Where the struct is in the value as it will be. |
| `path` | The name of the struct, as a refusal names it. |

## Constants

| Constant | Value |
| --- | --- |
| `ABI_STORAGE_NAMES` | The name of each storage layout as a refusal says it, by `enum bridge_storage`: a table, a sparse set, a toggle, direct-indexed, a singleton. |

## Functions

| Function | Source | Result |
| --- | --- | --- |
| `abi_hash_text` | `src/lib/abi/abi_stated.c` | The fingerprint of a text: `docs/intern/lib/abi/abi_stated.c.md`, *Functions*. |
| `abi_is_same_layout` | `src/lib/abi/abi_layout.c` | Whether a declaration matches its type: `docs/intern/lib/abi/abi_layout.c.md`, *Functions*. |
| `abi_is_listed` | `src/lib/abi/abi_names.c` | Whether a list holds a name: `docs/intern/lib/abi/abi_names.c.md`, *Functions*. |
| `abi_is_foreign` | `src/lib/abi/abi_names.c` | Whether a name is foreign: `docs/intern/lib/abi/abi_names.c.md`, *Functions*. |
| `abi_is_writable` | `src/lib/abi/abi_declare.c` | Whether a module may write a type: `docs/intern/lib/abi/abi_declare.c.md`, *Functions*. |
| `abi_find_type` | `src/lib/abi/abi_names.c` | Finds a type that a field names: `docs/intern/lib/abi/abi_names.c.md`, *Functions*. |
| `abi_say_others` | `src/lib/abi/abi_names.c` | Names the other declarers: `docs/intern/lib/abi/abi_names.c.md`, *Functions*. |
| `abi_accepts` | `src/lib/abi/abi_relay_run.c` | Whether a reload accepts a loss: `docs/intern/lib/abi/abi_relay_run.c.md`, *Functions*. |
| `abi_get_element_size` | `src/lib/abi/abi_relay_run.c` | The size of one element: `docs/intern/lib/abi/abi_relay_run.c.md`, *Functions*. |
| `abi_add_run` | `src/lib/abi/abi_relay_run.c` | Records a run: `docs/intern/lib/abi/abi_relay_run.c.md`, *Functions*. |
| `abi_say_lost` | `src/lib/abi/abi_relay_run.c` | Records a loss by name: `docs/intern/lib/abi/abi_relay_run.c.md`, *Functions*. |
| `abi_names_superseded` | `src/lib/abi/abi_moved.c` | Whether a field names a type that the reload made anew: `docs/intern/lib/abi/abi_moved.c.md`, *Functions*. |
| `abi_find_holder` | `src/lib/abi/abi_moved.c` | Finds a type that holds `c`: `docs/intern/lib/abi/abi_moved.c.md`, *Functions*. |
| `abi_check_move` | `src/lib/abi/abi_moved.c` | Checks a move to another storage layout: `docs/intern/lib/abi/abi_moved.c.md`, *Functions*. |
| `abi_relay_fields` | `src/lib/abi/abi_relay_fields.c` | Matches the fields of a struct: `docs/intern/lib/abi/abi_relay_fields.c.md`, *Functions*. |
| `abi_relay` | `src/lib/abi/abi_relay.c` | Works out how values carry over: `docs/intern/lib/abi/abi_relay.c.md`, *Functions*. |
| `abi_check_stated` | `src/lib/abi/abi_stated.c` | Checks what a declaration states: `docs/intern/lib/abi/abi_stated.c.md`, *Functions*. |
| `abi_read_layout` | `src/lib/abi/abi_layout.c` | Reads and checks the descriptor: `docs/intern/lib/abi/abi_layout.c.md`, *Functions*. |
| `abi_check_size` | `src/lib/abi/abi_layout.c` | Checks the stated size and alignment: `docs/intern/lib/abi/abi_layout.c.md`, *Functions*. 1 with a refusal. |
| `abi_replace_type` | `src/lib/abi/abi_replace.c` | Decides what becomes of an earlier type: `docs/intern/lib/abi/abi_replace.c.md`, *Functions*. |
| `abi_make_type` | `src/lib/abi/abi_make.c` | Makes a declared type: `docs/intern/lib/abi/abi_make.c.md`, *Functions*. |
