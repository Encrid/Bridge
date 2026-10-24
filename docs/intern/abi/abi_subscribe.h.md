# src/abi/abi_subscribe.h

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
| `abi_check_subscriber` | `src/abi/abi_subscriber.c` | Checks what a registration states: `docs/intern/abi/abi_subscriber.c.md`, *Functions*. |
| `abi_alloc_subscriber` | `src/abi/abi_subscriber_room.c` | Has the memory of a registration: `docs/intern/abi/abi_subscriber_room.c.md`, *Functions*. |
| `abi_free_subscriber` | `src/abi/abi_subscriber_room.c` | Frees what a refused registration had: `docs/intern/abi/abi_subscriber_room.c.md`, *Functions*. |
| `abi_check_term` | `src/abi/abi_term.c` | Checks one term: `docs/intern/abi/abi_term.c.md`, *Functions*. |
| `abi_check_before` | `src/abi/abi_before.c` | Checks the list of `before`: `docs/intern/abi/abi_before.c.md`, *Functions*. |
| `abi_read_before` | `src/abi/abi_before.c` | Reads the slots of `before`: `docs/intern/abi/abi_before.c.md`, *Functions*. |
| `abi_read_phase` | `src/abi/abi_phase.c` | Finds the phase of a system: `docs/intern/abi/abi_phase.c.md`, *Functions*. |
| `abi_subscribe` | `src/abi/abi_subscribe.c` | Registers a system or an observer: `docs/intern/abi/abi_subscribe.c.md`, *Functions*. |
