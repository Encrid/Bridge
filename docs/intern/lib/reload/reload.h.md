# src/lib/reload/reload.h

This header gives what the steps of a reload share: the reload in progress, and the functions of the steps.
`docs/intern/lib/bridge/bridge_reload.c.md`, *The steps of a reload*, tells their order.
It includes `src/lib/module/module.h`.

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
| `reload_refuse` | `src/lib/reload/reload_check.c` | Says why a module is not reloaded: `docs/intern/lib/reload/reload_check.c.md`, *Functions*. |
| `reload_check` | `src/lib/reload/reload_check.c` | Checks a reload: `docs/intern/lib/reload/reload_check.c.md`, *Functions*. |
| `reload_grow_share` | `src/lib/reload/reload_share.c` | Grows the share of the module: `docs/intern/lib/reload/reload_share.c.md`, *A share that a reload grows*. |
| `reload_give_back` | `src/lib/reload/reload_share.c` | Gives the growth back: `docs/intern/lib/reload/reload_share.c.md`, *Functions*. Nothing that lives was allocated from it. |
| `reload_make_record` | `src/lib/reload/reload_record.c` | Makes the new record: `docs/intern/lib/reload/reload_record.c.md`, *What the new record takes over*. |
| `reload_take_cursors` | `src/lib/reload/reload_record.c` | Takes the ids and the share of the old record into the new one: `docs/intern/lib/reload/reload_record.c.md`, *Functions*. |
| `reload_move_rows` | `src/lib/reload/reload_rows.c` | Moves the rows: `docs/intern/lib/reload/reload_rows.c.md`, *Functions*. |
| `reload_fail` | `src/lib/reload/reload_fail.c` | Undoes a reload that failed: `docs/intern/lib/reload/reload_fail.c.md`, *Functions*. |
| `reload_retire` | `src/lib/bridge/bridge_reload.c` | Takes the systems of the old instance aside, for one module or a group: `docs/intern/lib/bridge/bridge_reload.c.md`, *Functions*. |
| `reload_swap` | `src/lib/bridge/bridge_reload.c` | Puts a new record in the place of its old: `docs/intern/lib/bridge/bridge_reload.c.md`, *Functions*. |
| `reload_undo` | `src/lib/reload/reload_fail.c` | Takes a new instance back out: `docs/intern/lib/reload/reload_fail.c.md`, *Functions*. |
| `reload_set_move` | `src/lib/reload/reload_rows.c` | Writes the move of one type: `docs/intern/lib/reload/reload_rows.c.md`, *Functions*. |
| `reload_settle` | `src/lib/reload/reload_rows.c` | Settles a reloaded module after the rows moved: `docs/intern/lib/reload/reload_rows.c.md`, *Functions*. |
| `reload_refuse_move` | `src/lib/reload/reload_rows.c` | Writes why the rows did not move: `docs/intern/lib/reload/reload_rows.c.md`, *Functions*. |
| `reload_move_group` | `src/lib/reload/reload_group_rows.c` | Moves the rows of a group as one: `docs/intern/lib/reload/reload_group_rows.c.md`, *One move for the list*. |
| `reload_commit_group` | `src/lib/reload/reload_group_rows.c` | Commits a group reload: `docs/intern/lib/reload/reload_group_rows.c.md`, *The commit*. |
