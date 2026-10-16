# src/bridge_stop.c

This file stops a deck for the loss of a module, and lets the deck run again.
It also gives the stem of an identity, and a module pinned to another thread.

## A stopped deck

The host owns the process, so Bridge cannot stop the program.
When a module that states `fail` in `on_error` is lost, Bridge stops the deck: it refuses to go on.
A module is lost when its load fails, or when it is stopped at its failure budget.
It is lost too when its reload fails with no old instance to keep.

A stopped deck refuses each `bridge_run_phase`, and each load, unload and reload that does not name the module that stopped it.
The record that tells the stop names the module, and `bridge_list_modules` shows the module stopped.
A deck that has stopped stays stopped for its first reason.

A stopped deck accepts two calls, both about the module that stopped it:

- A load of the module, if it never loaded, or a reload of it. If that succeeds, the deck runs again, and a record says so.
- An unload of the module, by its name. The host so says that the deployment goes on without it. The deck runs again, and a record says so.

After that unload, the host can load any module, and a module of another name can take its place.
A module that never loaded has nothing to unload. For it, the ways on are a load that succeeds, or `bridge_destroy`.
Storage that runs out does not stop a deck: `docs/intern/block_barrier.c.md`, *The count of a module*.
The deck keeps why it stopped, and the stem of the module, in two arrays of its own: `docs/intern/inside.h.md`, *A deck*.

## The stem of an identity

The stem is the identity without its version: what comes before its `@`, or all of it if it has no `@`.
The stem tells the modules apart.
So a load or a reload of another version of the module that stopped the deck repairs the deck too.
An unload names the whole identity of the loaded module, with its version.

## Functions

| Function | Result |
| --- | --- |
| `bridge_get_stem_length` | Gives the length of an identity without its version: the bytes before its `@`, or all of them. |
| `bridge_find_pinned` | Gives the first loaded module that is pinned to a thread other than the calling thread, or null. |
| `bridge_stop` | Stops the deck for the loss of a module that states `fail`, and keeps why and the stem of the module. Does nothing if the deck has stopped already. |
| `bridge_resume` | Lets the deck run again, with a record, if `m` has the stem of the module that stopped it. |
| `bridge_give_up` | Lets the deck run again without the module that stopped it, with a record, if `m` has its stem. |
