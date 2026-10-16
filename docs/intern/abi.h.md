# src/abi.h

This header gives the ABI: the eight entries that a module imports.
It also gives what the rest of the library asks of the files of the ABI.

## The entries

A module calls three entries to register, in its first call: `init`, or `reload` at a reload.

```text
declare(at)       -> slot
add_system(at)    -> slot
observe(at)       -> slot
```

Each of the three takes the offset of one struct in the scratch arena of the module.
The structs are `bridge_declaration_t`, `bridge_system_statement_t` and `bridge_observer_statement_t`.
A name is allowed in them, because a registration is not on the hot path.

A module calls the other five entries at any time:

```text
spawn()                  -> id
destroy(id)
attach(id, slot, ptr)
detach(id, slot)
alloc(len)               -> offset into the persistent arena
```

Every pointer argument is an offset into the scratch arena of the caller: `docs/intern/abi_entry.h.md`, *The scratch arena*.
An entry reads what it is handed during the call, and the scratch that held it is free after the call.

Nothing applies at once.
`destroy`, `attach` and `detach` put a change on the queue of the module, and the next barrier applies it.
So a row exists at the next barrier, and not before.
`attach` carries the first value of the component: Bridge copies the value at the call, so the scratch arena can be reset before the barrier.
`spawn` gives a usable id at once: one of the ids set aside for the module at the last barrier.
So a system can spawn a parent and give its id to the children in the same call.

There are no immediate forms of these entries.
A span is a snapshot of the last barrier, so a row made at once is not reachable.
And the last row of a table fills the place of a row that leaves.
So an immediate destroy moves the row of another entity.
The offsets that a module holds then become wrong, and no check can find it.

No entry lets a module call another module.
A module that needs something done writes data, and the module that subscribes to that data does the work: a system or an observer.
To read and to write a component is not an entry either: a system reads its spans and writes its columns.
There is no entry for each frame: all the work of a frame is in systems, and each system has a phase.

Each entry is a `backend_fn_t`: it takes the module as `ud`, its arguments as 64-bit values in `a`, and writes its results in `r`.
`docs/intern/abi_define.c.md`, *The signatures*, gives the signature of each entry.

## What a pass depends on

A pass depends on storage, on the phase that the host asks for, and on what the native modules wrote.
Nothing else comes in: a value from outside reaches the other modules as a column that a native module writes, applied at a barrier.
Three facts make it so:

- No module can call another, so no value comes back as a result of a call.
- The code of a native module runs inside a phase.
- Two writers of one component never run at the same time, so the order of the others cannot change the result.

So the entries of a phase do not touch storage, and what a module asks for does not depend on who ran beside it.
An id comes from a block set aside for the module, arena space from its own share, and a change goes on its own queue.
The barrier applies the queues in an order that never changes.
So these are the same on each run: the ids that a module gets, its offsets in the arena, and the rows of its changes.

A timeout is the one thing that is not the same on each run.
The same `init` can end on a quiet machine and be stopped on a busy one.

No entry takes a lock.
The systems of a module run one after another, so its block of ids, its share and its queue each have one writer.

## A refused entry

An entry that cannot do what it is asked writes why in the record of the module, and gives non-zero.
The non-zero result stops the invocation that made the call: `docs/intern/abi_define.c.md`, *The signatures*, tells how a backend does it.
The invocation is then a failed system, or a failed first call: `docs/intern/module.h.md`, *When a call fails*.

The first refusal of a call is the reason, and Bridge keeps it.
A backend that cannot unwind a module lets the module run on, and what the module meets after the refusal is a consequence of it.
The reason becomes the failure of the invocation: `src/module_call.c` reads it for a lifecycle call, and `src/schedule_dispatch.c` for a system.

`spawn` gives an id and no status, so a module has nothing to test: the failure cannot be a value.
The same holds for each refusal, for example a `spawn` past `spawns`, or an `attach` of a slot never declared.
A module that spawns too much costs itself one invocation, and costs the other modules nothing.
A `declare` or an `add_system` past `[budget] types`, `systems` or `terms` fails the first call of the module.

## Types

| Type | Content |
| --- | --- |
| `abi_entry_t` | One import of the ABI: its `name`, `fn`, which is the function that does the entry, and its signature. |

The signature of an `abi_entry_t` is `params` and `param_count`, and `results` and `result_count`: at most 3 parameters and 1 result.

## Functions

| Function | Source | Result |
| --- | --- | --- |
| `abi_check_name` | `src/abi_stated.c` | Refuses a name that has 0 bytes, or more than 127. 1 with a refusal. |
| `abi_refuse` | `src/abi_alloc.c` | Refuses the call in progress, with why, written as `printf` writes. Gives 1. |
| `abi_define` | `src/abi_define.c` | Defines the eight imports for the instance of a module. 1, with `err`, if its backend refuses one. |
| `abi_marks` | `src/abi_alloc.c` | Makes room for the marks of the queue of a module, so that a queued change never asks for memory. False if there is no memory. |
| `abi_park` | `src/abi_alloc.c` | Takes a system out of storage, for a reload. |
| `abi_unpark` | `src/abi_alloc.c` | Puts a parked system back in storage, as it was. |
| `abi_hear` | `src/abi_system.c` | Adds a change that an observer is owed to the list of who triggered it. Storage calls it. |
| `abi_declare` | `src/abi_declare.c` | `declare(at) -> slot`. |
| `abi_add_system` | `src/abi_system.c` | `add_system(at) -> slot`. |
| `abi_observe` | `src/abi_system.c` | `observe(at) -> slot`. |
| `abi_spawn` | `src/abi_calls.c` | `spawn() -> id`. |
| `abi_destroy` | `src/abi_calls.c` | `destroy(id)`. |
| `abi_attach` | `src/abi_calls.c` | `attach(id, slot, ptr)`. |
| `abi_detach` | `src/abi_calls.c` | `detach(id, slot)`. |
| `abi_alloc` | `src/abi_alloc.c` | `alloc(len) -> offset`. |
