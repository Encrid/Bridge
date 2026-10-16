# src/bridge_run.c

This file runs a phase, checks each call that runs the code of a module, and tells what is loaded.

## A phase run

`bridge_run_phase` runs the systems of one phase, then the barrier of that phase.
The loop is the host's: it calls the phases that it wants, in its order, as often as it wants.
Bridge has no idea of a complete pass, so it cannot see a phase that the host forgot.
A phase with no system runs nothing, and nothing in Bridge tells that a program is done.
A phase is a string, and `bridge_run_phase` takes its name each time.
It compares the name with the few phases of the deployment, and refuses a name that is not one of them, by that name.

## A call that runs the code of a module

`bridge_run_phase`, `bridge_load`, `bridge_reload` and `bridge_unload` each start with `bridge_check_call`.
A stopped deck refuses the call, unless the call names the module that stopped the deck: `docs/intern/bridge_stop.c.md`, *A stopped deck*.
The call compares the identities without their versions, so another version of that module can repair the deck too.
A loaded module that is pinned to another thread refuses the call too.

## A pinned module

A module that states `pinned` runs each of its systems and observers on the thread that its `init` ran on.
Some code must run on the thread that created what it drives.
Some code needs one thread in particular, for example the first thread of the process, and only the host can choose that thread.
So `init` runs inside `bridge_load`, on the thread that called it.
The systems of a pinned module run on the thread that calls `bridge_run_phase`.
A host with a pinned module loaded calls both from one thread.

The author of the module states `pinned`, because only the author knows that the code needs one thread.
Each call from another thread is refused by name, before anything runs, and so is `bridge_destroy`.
The cost is that all pinned modules run one after another, on that one thread.
A module that needs a thread other than the thread of the host has no way to state it.
Nothing is sent to another thread, and nothing is deferred: the call on the wrong thread never runs.

## What is loaded

`bridge_list_modules` tells each loaded module, and whether it still runs.
It exists because a module that loaded can stop later, past its failure budget, and nothing else says so.
It also tells why the deck stopped: the module that stopped it shows as stopped.
A module whose reload failed with no old instance to keep shows as stopped too.
It is a pull, not a record, because the log can drop a record.
A state must not arrive by a route that can drop it.

It gives two fields for each module: its identity and its state. The host knows the rest, since the host gave it.
A module that is not loaded is not in the list.
It writes at most `max` modules into `out`, and gives the number of all the loaded modules.

## Functions

| Function | Result |
| --- | --- |
| `is_repair` | Gives whether a call names the module that stopped the deck, by its identity without the version. |
| `bridge_check_call` | 1, with a record, if nothing may run: a stopped deck that the call does not repair, or a module pinned to another thread. |
| `find_phase` | Gives the index of the phase of a name, or the number of phases if the name is none of them. |
| `bridge_run_phase` | Runs the systems of one phase, then its barrier. 0 if the phase ran, whatever its systems did, and 1, with a record, if refused. |
| `bridge_list_modules` | Writes up to `max` loaded modules into `out`, each with its state, and gives how many modules are loaded. |
