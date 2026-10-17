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
| `reload_refuse` | `src/reload_check.c` | Says why a module is not reloaded: `docs/intern/reload_check.c.md`, *Functions*. |
| `reload_check` | `src/reload_check.c` | Checks a reload: `docs/intern/reload_check.c.md`, *Functions*. |
| `reload_grow_share` | `src/reload_share.c` | Grows the share of the module: `docs/intern/reload_share.c.md`, *A share that a reload grows*. |
| `reload_give_back` | `src/reload_share.c` | Gives the growth back: `docs/intern/reload_share.c.md`, *Functions*. Nothing that lives was allocated from it. |
| `reload_make_record` | `src/reload_record.c` | Makes the new record: `docs/intern/reload_record.c.md`, *What the new record takes over*. |
| `reload_move_rows` | `src/reload_rows.c` | Moves the rows: `docs/intern/reload_rows.c.md`, *Functions*. |
| `reload_fail` | `src/reload_fail.c` | Undoes a reload that failed: `docs/intern/reload_fail.c.md`, *Functions*. |
