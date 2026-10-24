# src/module/module_record.c

This file makes the record of a module from its config: all of it, or none of it.

## All or none

The record asks for every part that it is made of first.
If one part is missing, it keeps none of them.
Then a record says "out of memory: there was none for a module's record", and the parts that were had are freed.
The general rule is in `docs/intern/say/say.h.md`, *Out of memory*.

## What the record holds

| Part | Content |
| --- | --- |
| What was stated | `pinned`, `on_error`, `hot_reload`, `max_system_failures`, `shutdown_timeout`, `work_limit`, `spawns`, `persistent` and `loss_count`. |
| The thread | The thread that makes the record, which is the thread that loads the module. |
| The names | Copies of the identity, of the stem, and of the namespace: the text before the colon. An identity has one colon: `docs/intern/module/module_identity.c.md`. |
| The losses | The `losses` of the config itself, and a count of rows for each loss. |
| The ids | Room for `spawns` ids. |
| The queue | Room for `changes` bytes of changes. |
| The phase names | Each name that the module uses, and the index of the phase of the deployment that it stands for. |
| The grants | Copies of `read` and `write`. |

The room for the ids and for the queue is had now, at the load, as long as the config states.
So no phase asks for memory to spawn or to queue: `docs/intern/inside.h.md`, *Nothing is allocated in a phase*.
A `loss_count` of `BRIDGE_NONE` is 0 in the record.
The record keeps a pointer to the `losses` of the config, which the load or the reload reads while it runs.
The checks found the phase of each name already, so the search for its index always ends.

## Functions

| Function | Result |
| --- | --- |
| `copy_list` | Gives a copy of a list of texts, or `nullptr` with nothing kept if there was not the memory. |
| `copy_stated` | Copies into the record what the config states, and the thread that loads the module. |
| `alloc_room` | Copies the names, and makes the room for the ids and the queue as long as the config states. False if one part is missing. |
| `copy_phases` | Copies the phase names of the module, each with the phase of the deployment that it stands for. False if there was not the memory. |
| `module_record` | Makes the record of a module from its config: all of it, or none. Gives `nullptr` with a record said. |
