# src/abi_subscribe.h

This header gives what the files that register a system or an observer share.
A system and an observer are each a query and a function, so one registration serves both.

## Types

| Type | Content |
| --- | --- |
| `subscribing_t` | A system or an observer being registered. |

### subscribing_t

| Field | Content |
| --- | --- |
| `m` | The module that registers. |
| `name_at`, `name_len` | Where the module put the name. |
| `terms_at`, `count` | The terms, each a `bridge_term_t`. |
| `changes_at`, `change_count` | The slots of what it attaches or detaches, each a `u32`. |
| `before_at`, `before_count` | The slots of the systems before it, each a `u32`. |
| `phase` | The phase that a system named, or for an observer the phase past the last of the deployment. |
| `events` | Zero for a system, and the events for an observer. |
| `slot` | The slot that it gets: how many systems the module has so far, or how many observers. |
| `what` | `system` or `observer`, as a refusal says it. |
| `name` | The name, copied. |
| `terms`, `changes`, `before` | The terms, the changes and the slots of `before`, found in the scratch arena. A module can put them at any offset. |
| `of` | The term being read, as a refusal names it. |
| `s` | The system that is made of it. |
| `asked` | What storage is asked: one `storage_term_t` for each term. |

## Functions

| Function | Source | Result |
| --- | --- | --- |
| `abi_check_subscriber` | `src/abi_subscriber.c` | Refuses a system or an observer past a budget, with what lies outside the scratch arena, or with a reserved or taken name. Copies the name. 1 with a refusal. |
| `abi_alloc_subscriber` | `src/abi_subscriber_room.c` | Has everything that a system or an observer needs, before it keeps any of it. 1 with a refusal, and nothing left allocated. |
| `abi_free_subscriber` | `src/abi_subscriber_room.c` | Frees what `abi_alloc_subscriber` had, of a system or an observer that is refused. |
| `abi_check_term` | `src/abi_term.c` | Refuses term `i` if it asks for what a query may not. 1 with a refusal. |
| `abi_check_before` | `src/abi_before.c` | Finds the list of `before`, and refuses it outside the scratch arena or with more slots than the module has earlier systems. 1 with a refusal. |
| `abi_read_before` | `src/abi_before.c` | Reads the slots of `before`, and gives the new system the bits of each one. 1 with a refusal. |
| `abi_read_phase` | `src/abi_phase.c` | Finds the phase that a system names, and refuses a name that would merge two phases of the module. 1 with a refusal. |
| `abi_subscribe` | `src/abi_subscribe.c` | Registers a system or an observer, and gives its slot in `r`. 1 with a refusal. |
