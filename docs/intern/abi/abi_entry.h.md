# src/abi/abi_entry.h

This header gives what the entries of a phase share.
It reads the scratch arena with a bounds check, and puts a change on the queue of a module.
It also checks whether the module may make the change.

## The scratch arena

`get_scratch` gives the `len` bytes at offset `at` of the private memory of a module, if they lie in its scratch arena.
If not, it gives nullptr, and the entry refuses the call.
An offset that a module hands over must lie in the scratch arena: `docs/intern/block/block_log.c.md`, *The log ring*, tells why.

## The queue

The queue of a module is as long as the module stated in `changes`, and it is made when the module loads.
A change on the queue is a `change_t` of 24 bytes, then its value, padded to a multiple of eight bytes.
The value is copied at the call, since the scratch arena that holds it is reset before the barrier applies the change.
`queue_change` asks for no memory: `docs/intern/inside.h.md`, *Nothing is allocated in a phase*.

A module that stated `BRIDGE_NONE` for `changes` has a queue with no room, and each change it asks for is refused.
`queue_change` refuses a change when the rest of the queue has no room for it: the changes of the module are used up.

The first change of a turn puts a mark in the queue: where the changes of that turn begin.
The barrier uses these marks to keep the order of the calls: `docs/intern/storage/storage_runs.h.md`, *The order of the queues*.
`abi_marks` makes the room for the marks before a turn: `docs/intern/abi/abi_alloc.c.md`, *The marks of the queue*.
Between two applies, a module takes at most one mark for each system, one for each observer, and one for the lifecycle call.
Each apply empties the marks, so the room is always there. `mark_turn` guards it still: a mark with no room is refused with a record.

## What a call may change

What a system attaches or detaches is what it stated in its changes.
So two systems that change one component were put in order when they loaded: see `src/schedule/schedule_conflict.c`.
A lifecycle call runs alone and states nothing, so `is_stated` lets it change any component that it may write.

A singleton is on no entity, so a module attaches and detaches it with the id 0, which is no entity.
No other component takes the id 0.
`check_owner` refuses a change of a singleton with another id, and a change of any other component with the id 0.

## Functions

| Function | Source | Result |
| --- | --- | --- |
| `get_scratch` | `src/abi/abi_entry.h` | The `len` bytes at `at` in the scratch arena of the module, or nullptr if they do not lie in it. |
| `mark_turn` | `src/abi/abi_entry.h` | Marks where the changes of the turn in progress begin in the queue, at the first change of that turn. Refuses with a record if the marks have no room. |
| `queue_change` | `src/abi/abi_entry.h` | Adds a change to the queue of the module, with a copy of its value after it. 1 if refused. |
| `is_stated` | `src/abi/abi_entry.h` | Whether the system or the observer that runs states the component among its changes. True for a lifecycle call. |
| `check_owner` | `src/abi/abi_entry.h` | Refuses a change of a singleton that names an entity, and a change of anything else that names none. 1 if refused. |
