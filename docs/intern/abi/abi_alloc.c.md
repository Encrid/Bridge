# src/abi/abi_alloc.c

This file gives the refusal of an entry, and the persistent arena handed out to a module.
It also gives the room for the marks of a queue, and a system taken out of storage for a reload.

## A refusal

`abi_refuse` writes why the call in progress is refused into `why` of the module, and sets `refused`.
A second refusal in the same call changes nothing: the first is the reason.
`docs/intern/abi/abi.h.md`, *A refused entry*, tells what the refusal does.

## The persistent arena

`docs/extern/bridge_abi.h.md`, *alloc*, tells what `alloc(len)` gives.
Each module has a share of its own, so its offsets do not depend on the order in which the threads of other modules ran.
The share is one or more ranges, in `owned`.
The load gives one range, and a reload can add more: `docs/intern/reload/reload_share.c.md`, *A share that a reload grows*.
`alloc` goes to the next range when the request does not fit in what is left of the current one.
What is left of the range before is dead space, as a string that outgrew its place leaves.

Nothing frees what a module allocated while the module is loaded, and no entry frees or resets the arena.
A string can be in a component that another module reads, and a reset would make the offset that the other module holds wrong.
At unload, the share goes back to the arena if no row points into it: `docs/intern/module/module_share.c.md`, *When a share goes back*.

`alloc` is refused for a module that stated `BRIDGE_NONE` for `persistent`: it keeps nothing in the arena.
`alloc` is refused when the request does not fit in the share: the `persistent` of the module is used up.

## The marks of the queue

A mark says where in the queue of a module the changes of one turn begin.
Each call that puts a change on the queue takes one mark.
`abi_marks` makes room for one mark from each system and each observer of the module, and one from the lifecycle call in progress.
It adds these to the marks that the module has, with `more` beyond them.
`src/abi/abi_subscriber_room.c` asks for this room when a module gains a system, and `src/module/module_call.c` asks for it before a lifecycle call.
So a change that is put on the queue never has to ask for memory.

## A parked system

At a reload, the systems of a module count one time against `[budget] systems`: `docs/extern/bridge.h.md`, *bridge_budget_t*.
`src/bridge/bridge_reload.c` parks each system of the old instance: storage forgets its terms, and the world counts it in `parked`.
A parked system counts for nothing against `[budget] systems`, so the systems of the new instance take the places that the old ones had.
If the reload fails, `src/reload/reload_fail.c` puts each parked system back, as it was.

Each reload ends with `swap` or with `src/reload/reload_fail.c`, and neither leaves a system parked.
So no system is parked when `abi_park` parks it, and `abi_park` does not test it.
The places it had are free again, since whatever took them was dropped.

## Functions

| Function | Result |
| --- | --- |
| `abi_refuse` | Refuses the call in progress, with why, written as `printf` writes. Keeps only the first refusal of a call. Gives 1. |
| `abi_alloc` | `alloc(len) -> offset`: `len` bytes, rounded up to eight, from the share of the module, or of its `lender`, in the persistent arena. |
| `abi_marks` | Makes room for the marks of the queue of a module. False if there is no memory. |
| `abi_park` | Takes a system out of storage, for a reload. |
| `abi_unpark` | Puts a parked system back in storage: an observer with its listener, a system with its terms. A system with no term stays out of storage. |
