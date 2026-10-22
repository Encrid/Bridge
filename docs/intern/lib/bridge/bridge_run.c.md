# src/lib/bridge/bridge_run.c

This file runs a phase, and tells what is loaded.

## A phase run

`bridge_run_phase` runs the systems of one phase, then the barrier of that phase.
The loop is the host's: it calls the phases that it wants, in its order, as often as it wants.
Bridge has no idea of a complete pass, so it cannot see a phase that the host forgot.
A phase with no system runs nothing, and nothing in Bridge tells that a program is done.
A phase is a string, and `bridge_run_phase` takes its name each time.
It compares the name with the few phases of the deployment, and refuses a name that is not one of them, by that name.

## What is loaded

`bridge_list_modules` tells each loaded module, and whether it still runs.
It exists because a module that loaded can stop later, past its failure budget, and nothing else says so.
It also tells why the world stopped: the module that stopped it shows as stopped.
`docs/extern/lib/bridge.h.md`, *bridge_module_info_t*, tells each cause of a stop.
It is a pull, not a record, because the log can drop a record.
A state must not arrive by a route that can drop it.

It gives two fields for each module: its identity and its state. The host knows the rest, since the host gave it.
A module that is not loaded is not in the list.
It writes at most `max` modules into `out`, and gives the number of all the loaded modules.

## Functions

| Function | Result |
| --- | --- |
| `bridge_run_phase_now` | Runs the systems of one phase, then its barrier. Its result is in `docs/extern/lib/bridge.h.md`, *bridge_run_phase*. |
| `bridge_list_modules_now` | Writes up to `max` loaded modules into `out`, each with its state, and gives how many modules are loaded. |
