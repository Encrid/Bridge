# src/say.h

This header gives the records of the library's own, and what each file of the library says when there is no memory.

## Out of memory

An allocation that fails is a refusal like any other.
The call that needed the memory fails. A record says "out of memory" and what for, and the deck stays as it was.
`bridge_lacks` writes that record.

The library asks the allocator for memory when a deck is made, and when a module is loaded, reloaded or unloaded.
It never asks in a phase or at a barrier: `docs/intern/inside.h.md`, *Nothing is allocated in a phase*.
So the call that fails is `bridge_create`, `bridge_load` or `bridge_reload`, by the path that each has already.
The deck is not made, the module is not loaded, or the old instance runs on after the reload, where the backend can keep it.

Whatever a call needs, it gets before it keeps any of it.
A type, a system and the record of a module each ask for all of their parts first.
If one part is missing, they keep none, so nothing half made is left to unmake.

An unload does not fail for want of memory, because it asks for no memory that it cannot do without.
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
| `say_error` | `src/say.c` | Hands the sink an error of the library's own, about no module, written as `printf` writes. |
| `say_about` | `src/say.c` | Hands the sink a record of the library's own about a module or one of its systems, at the level that `about` states. |
| `copy_text` | `src/say.c` | Gives a copy of the first `n` bytes of a text, with a zero after them. Null if there was not the memory: the caller says so. |
| `bridge_lacks` | `src/say.c` | Gives 1, with a record that tells what for, if `got` is null: there was no memory for `what`. Gives 0 if not. |
