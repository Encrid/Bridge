# src/bridge_check.c

This file checks each call of a function of the world, before the call changes anything.

## A call from the sink

Bridge calls the sink inside a function of the world, while that function is in the middle of its work: `docs/intern/say_sink.c.md`.
A second call of the world from the sink would run on that work, for example free a module that a drain still reads.
A module gets no world, so only the sink calls the world on the thread whose call runs.
Then `bridge_enter` finds the id of its own thread in `caller`, and does not claim the world: `owner` stays false.
Each function but `bridge_list_modules` then gives 1 and writes no record, since a record calls the sink again.

This test comes first. Else a sink that calls on a stopped world would get a record, and call again with no end.
`bridge_list_modules` only reads, so the sink can call it.

## A call from another thread

Each function of the world claims the world with `bridge_enter` before it reads or changes anything.
It gives the world back with `bridge_leave` when it returns, and `bridge_destroy` gives nothing back, since it frees the world.
The claim is one atomic compare-exchange of `caller`: 0 becomes the id of the calling thread, and another value stays.
A call that finds the id of its own thread comes from the sink: *A call from the sink*.
A call that finds the id of another thread does nothing and writes no record, since a record would call the sink on its thread.
The claim acquires and the giving back releases, so the next call sees what the call before it wrote.

## A call that runs the code of a module

`bridge_run_phase`, `bridge_load`, `bridge_reload` and `bridge_unload` each start with `bridge_check_call`.
`bridge_check_call` starts with `bridge_check_entry`, and `bridge_destroy` calls `bridge_check_entry` alone.
A stopped world refuses the call, unless the call names the module that stopped the world: `docs/intern/bridge_stop.c.md`, *A stopped world*.
The call compares the identities without their versions, so another version of that module can repair the world too.
A loaded module that is pinned to another thread refuses the call too.

## A pinned module

A module that states `pinned` runs each of its systems and observers on the thread that its `init` ran on.
Some code must run on the thread that created what it drives.
Some code needs one thread in particular, for example the first thread of the process, and only the host can choose that thread.
So `init` runs inside `bridge_load`, on the thread that called it.
A host with a pinned module loaded calls `bridge_load` and `bridge_run_phase` from one thread: `docs/extern/bridge.h.md`, *A pinned module*.

The author of the module states `pinned`, because only the author knows that the code needs one thread.
Each call from another thread is refused by name, before anything runs, and so is `bridge_destroy`.
The cost is that all pinned modules run one after another, on that one thread.
A module that needs a thread other than the thread of the host has no way to state it.
Nothing is sent to another thread, and nothing is deferred: the call on the wrong thread never runs.

## Functions

| Function | Result |
| --- | --- |
| `is_repair` | Gives whether a call names the module that stopped the world, by its identity without the version. `stopped_stem` is set whenever `stopped_by` is. |
| `bridge_enter` | Claims the world for the calling thread. Gives false if another thread has it; sets `owner` if this call took it. |
| `bridge_leave` | Gives the world back, if `owner` says that this call took it. |
| `bridge_check_entry` | 1, with a record, if a module is pinned to another thread. Else 0. |
| `bridge_check_call` | 1 if `bridge_check_entry` refuses, or, with a record, if the world stopped and the call does not repair it. Else 0. |
