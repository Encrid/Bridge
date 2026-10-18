# src/lib/bridge/bridge_stop.c

This file stops a world for the loss of a module, and lets the world run again.
It also gives the stem of an identity, and a module pinned to another thread.

## A stopped world

The host owns the process, so Bridge cannot stop the program.
When a module that states `fail` in `on_error` is lost, Bridge stops the world: it refuses to go on.
A module is lost when its load fails, or when it is stopped at its failure budget.
It is lost too when its reload fails with no old instance to keep.

A stopped world refuses each `bridge_run_phase`, and each load, unload and reload that does not name the module that stopped it.
The record that tells the stop names the module, and `bridge_list_modules` shows the module stopped.
A world that has stopped stays stopped for its first reason.

A stopped world accepts two calls, both about the module that stopped it:

- A load of the module, if it never loaded, or a reload of it. If that succeeds, the world runs again, and a record says so.
- An unload of the module, by its name. The host so says that the deployment goes on without it. The world runs again, and a record says so.

After that unload, the host can load any module, and a module of another name can take its place.
A module that never loaded has nothing to unload. For it, the ways on are a load that succeeds, or `bridge_destroy`.
Storage that runs out is no cause of a stop: `docs/intern/lib/block/block_barrier.c.md`, *The count of a module*.
The world keeps why it stopped, and the stem of the module, in two arrays of its own: `docs/intern/lib/inside.h.md`, *A world*.

While the world is stopped, only a call that names that stem passes `bridge_check_call`.
So the module that `bridge_resume` or `bridge_give_up` gets has that stem, and they test only that the world stopped.

## The stem of an identity

The stem is the identity without its version: what comes before its `@`, or all of it if it has no `@`.
The stem tells the modules apart.
So a load or a reload of another version of the module that stopped the world repairs the world too.
An unload names the whole identity of the loaded module, with its version.

## Functions

| Function | Result |
| --- | --- |
| `bridge_get_stem_length` | Gives the length of an identity without its version: the bytes before its `@`, or all of them. |
| `bridge_find_pinned` | Gives the first loaded module that is pinned to a thread other than the calling thread, or null. |
| `bridge_stop` | Stops the world for the loss of a module that states `fail`, and keeps why and the stem of the module. Does nothing if the world has stopped already. |
| `bridge_resume` | Lets the world run again, with a record, if it stopped. |
| `bridge_give_up` | Lets the world run again without the module that stopped it, with a record, if it stopped. |
