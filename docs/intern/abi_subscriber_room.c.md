# src/abi_subscriber_room.c

This file has the memory of a system or an observer.
It has everything that the system needs before it keeps any of it: `docs/intern/say.h.md`, *Out of memory*.

## The memory

- One block for the system, its terms, and after the terms its changes.
- One `storage_term_t` for each term, plus one: what storage is asked.
- Room for one more among the systems of the world, and among the systems or the observers of the module.
- The name of the system, and its full name.
- For an observer, its list of who triggered it.
- Room for one more mark in the queue of the module: `docs/intern/abi_alloc.c.md`, *The marks of the queue*.

The full name is how `[order]` names a system: the stem of the module, a colon, and the name.
The stem is the identity of the module with no version.
The list of an observer has a place for as many `bridge_trigger_t` as the spans of its module can hold.
That is what the spans could publish of it: `docs/intern/abi_system.c.md`, *The list of an observer*.

If a piece is missing, all of it is freed, and the registration is refused for want of memory.

## Functions

| Function | Result |
| --- | --- |
| `abi_free_subscriber` | Frees what `abi_alloc_subscriber` had, of a system or an observer that is refused. |
| `make_room` | Makes room for one more among the systems of the world, and among the systems or the observers of the module. False if there is no memory. |
| `name_system` | Gives a system its name and its full name, and an observer its list of who triggered it. False if there is no memory. |
| `abi_alloc_subscriber` | Has everything that a system or an observer needs. 1 with a refusal, and nothing left allocated. |
