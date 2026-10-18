# src/lib/say/say.h

This header gives the records of the library's own, and what each file of the library says when there is no memory.

## Out of memory

An allocation that fails is a refusal like any other.
The call that needed the memory fails. A record says "out of memory" and what for, and the world stays as it was.
`bridge_lacks` writes that record.

The library asks the allocator for memory when a world is made, and when a module is loaded, reloaded or unloaded.
It never asks in a phase or at a barrier: `docs/intern/lib/inside.h.md`, *Nothing is allocated in a phase*.
So the call that fails is `bridge_create`, `bridge_load` or `bridge_reload`, by the path that each has already.
The world is not made, the module is not loaded, or the old instance runs on after the reload, where the backend can keep it.

Whatever a call needs, it gets before it keeps any of it.
A type, a system and the record of a module each ask for all of their parts first.
If one part is missing, they keep none, so nothing half made is left to unmake.

An unload never fails for want of memory, because it asks for no memory that it cannot do without.
The schedule is made again in room made at create.
A range of the arena given back goes into a list made long enough when the range was taken.
The one thing that an unload can do without is the place of the settings text of the module.
Without it, the next load of the module takes fresh room, and a record says so.

## Types

| Type | Content |
| --- | --- |
| `say_t` | Whose a record is about, when it is about somebody, and how bad it is: the module `m`, the system `s` and the `level`. |

## Functions

| Function | Source | Result |
| --- | --- | --- |
| `say_error` | `src/lib/say/say.c` | Says an error about no module: `docs/intern/lib/say/say.c.md`, *Functions*. |
| `say_about` | `src/lib/say/say.c` | Says a record about a module: `docs/intern/lib/say/say.c.md`, *A record of the library's own*. |
| `copy_text` | `src/lib/say/say.c` | Copies a text: `docs/intern/lib/say/say.c.md`, *Functions*. A caller that gets null says so. |
| `bridge_lacks` | `src/lib/say/say.c` | Says that there was no memory: `docs/intern/lib/say/say.c.md`, *Functions*. |
