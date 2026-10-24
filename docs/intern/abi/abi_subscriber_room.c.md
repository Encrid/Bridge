# src/abi/abi_subscriber_room.c

This file has the memory of a system or an observer.
It has everything that the system needs before it keeps any of it: `docs/intern/say/say.h.md`, *Out of memory*.

## The memory

- One block for the system, its terms, after the terms its changes, and after the changes the bits of `stated_before`: `docs/intern/schedule/schedule_before.c.md`.
- One `storage_term_t` for each term, plus one: what storage is asked.
- Room for one more system of the world, and for one more system or observer of the module.
- The name of the system, and its full name.
- For an observer, its list of who triggered it.
- Room for one more mark in the queue of the module: `docs/intern/abi/abi_alloc.c.md`, *The marks of the queue*.

The full name is how `[order]` names a system: the stem of the module, a colon, and the name.
The stem is the identity of the module with no version.
The system also gets `named`, the index of its full name among the names of `[order]`: `docs/intern/schedule/schedule_names.c.md`.
The list of an observer has a place for as many `bridge_trigger_t` as the spans of its module can hold.
That is what the spans could publish of it: `docs/intern/abi/abi_system.c.md`, *The list of an observer*.

`abi_alloc_subscriber` points `stated_before` of the system to its bits.
The bits start on a word of 8 bytes, since a term and a pointer each take a multiple of 8 bytes. Two static assertions say so.
If a piece is missing, all of it is freed, and the registration is refused for want of memory.

## Functions

| Function | Result |
| --- | --- |
| `abi_free_subscriber` | Frees what `abi_alloc_subscriber` had, of a system or an observer that is refused. |
| `make_room` | Makes room for one more system of the world, and for one more system or observer of the module. False if there is no memory. |
| `name_system` | Gives a system its name, its full name and `named`, and an observer its list of who triggered it. False if there is no memory. |
| `abi_alloc_subscriber` | Has everything that a system or an observer needs. 1 with a refusal, and then it holds no memory. |
