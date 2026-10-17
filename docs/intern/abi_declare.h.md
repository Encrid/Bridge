# src/abi_declare.h

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
| `c` | The type of that name that is there, and then the type declared. |
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
| `abi_hash_text` | `src/abi_stated.c` | The fingerprint of `n` bytes of text. |
| `abi_is_same_layout` | `src/abi_layout.c` | Whether a declared type has the kind and the descriptor that a declaration states. |
| `abi_is_listed` | `src/abi_names.c` | Whether a list of names holds a name. |
| `abi_is_foreign` | `src/abi_names.c` | Whether a name is of another namespace than the module's. |
| `abi_is_writable` | `src/abi_declare.c` | Whether a module may write a type. |
| `abi_find_type` | `src/abi_names.c` | Finds the type that a field names bare. The descriptor reader calls it, with a `declaring_t` as `ctx`. |
| `abi_say_others` | `src/abi_names.c` | Writes who else declares a type, as the end of a refusal. Nothing if nobody does. |
| `abi_accepts` | `src/abi_relay_run.c` | Whether a reload accepts the loss named `name`, which it now makes, of what `rows` rows hold. |
| `abi_get_element_size` | `src/abi_relay_run.c` | How many bytes one element of a field is. |
| `abi_add_run` | `src/abi_relay_run.c` | Records that `len` bytes at `from` of a value as it was are the bytes at `to` of the value as it will be. |
| `abi_say_lost` | `src/abi_relay_run.c` | Records by name what has no place in what the type becomes. |
| `abi_names_superseded` | `src/abi_moved.c` | Whether a field of a type names a type that this reload made anew. |
| `abi_find_holder` | `src/abi_moved.c` | A type with a field of type `c` that another loaded module declares, and that module in `by`. nullptr if none. |
| `abi_check_move` | `src/abi_moved.c` | Refuses a move to another storage layout that cannot be made, or that loses what is not accepted. 1 with a refusal. |
| `abi_relay_fields` | `src/abi_relay_fields.c` | Matches the fields of a struct as it was to those of what it becomes, by name. 1 with a refusal. |
| `abi_relay` | `src/abi_relay.c` | Works out how the values of a type are carried over to what it becomes, and keeps that on the type. 1 with a refusal. |
| `abi_check_stated` | `src/abi_stated.c` | Refuses a declaration that states what it may not, and works out its fingerprint. 1 with a refusal. |
| `abi_read_layout` | `src/abi_layout.c` | Reads and lays out the descriptor, and checks its size and alignment. 1 with a refusal, and nothing left allocated. |
| `abi_check_size` | `src/abi_layout.c` | Refuses a size and an alignment that the module states and that the descriptor does not give. 1 with a refusal. |
| `abi_replace_type` | `src/abi_replace.c` | Decides what becomes of the type of that name that is there already. 1 with a refusal. |
| `abi_make_type` | `src/abi_make.c` | Makes the type that a declaration states. 1 with a refusal. |
