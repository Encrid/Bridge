# src/reload.h

This header gives what the steps of a reload share: the reload in progress, and the functions of the steps.
`docs/intern/bridge_reload.c.md`, *The steps of a reload*, tells their order.
It includes `src/module.h`.

## Types

### reloading_t

A reload in progress.

| Field | Content |
| --- | --- |
| `b`, `a` | The world, and the artifact of the new instance. |
| `on`, `demands` | The backend that the artifact names, and what its config asks that backend to keep. |
| `old`, `n` | The record of the loaded instance, and the record of the instance that takes its place. |
| `extra` | How many bytes the share of the persistent arena grows by. |
| `in_place`, `last` | That the last range of the share grew where it is, and that range as it was. |
| `extra_at` | Where the added bytes are when they are a range more, from the start of the arena. |
| `side_by_side` | That the backend can hold both instances at once. |
| `err` | Why the reload failed. |

## Functions

| Function | Source | Result |
| --- | --- | --- |
| `reload_refuse` | `src/reload_check.c` | Says that the loaded module is not reloaded, and why, written as `printf` writes. Gives 1. |
| `reload_check` | `src/reload_check.c` | Refuses a reload of a module that is not loaded, that may not be reloaded, or that names another backend. 1 with a record. |
| `reload_grow_share` | `src/reload_share.c` | Grows the share of the module of the persistent arena to what the reload states, and never makes it smaller. 1 with a record. |
| `reload_give_back` | `src/reload_share.c` | Gives back what `reload_grow_share` added. Nothing that lives was allocated from it. |
| `reload_make_record` | `src/reload_record.c` | Makes the record of the new instance, which takes over what was set aside for the module. 1 with a record, and the old instance untouched. |
| `reload_move_rows` | `src/reload_rows.c` | Moves the rows of each type that the reload made in the place of another, all of them or none. 1 and why if not. |
| `reload_fail` | `src/reload_fail.c` | Says that the reload failed and takes the new instance out. Keeps the old instance, or else stops the module. Gives 1. |
