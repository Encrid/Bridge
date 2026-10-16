# src/bridge_inside.h

This header gives what the files that make, stop and run a world share, and what the rest of the library asks of them.
It includes `src/inside.h`.

`bridge_check_call` takes `repairing`: the module that a load, a reload or an unload names.
A stopped world accepts only a call that names the module that stopped it: `docs/intern/bridge_stop.c.md`, *A stopped world*.

## Functions

| Function | Source | Result |
| --- | --- | --- |
| `bridge_open_backends` | `src/bridge_backends.c` | Opens and starts each backend of the deployment, and takes what each needs of a block that it maps. 1, with a record, if one fails. |
| `bridge_open_backend` | `src/bridge_backend.c` | Opens and starts the backend `i`. 1, with a record, if it has no name or no library, has the name of another, does not open or does not start. |
| `bridge_name_phases` | `src/bridge_phases.c` | Takes the phases of the deployment, each a name. 1, with a record, if there is none, or a phase has no name or the name of another. |
| `bridge_get_stem_length` | `src/bridge_stop.c` | Gives the length of an identity without its version: the bytes before its `@`. |
| `bridge_find_pinned` | `src/bridge_stop.c` | Gives a loaded module that is pinned to a thread other than the calling thread, or null. |
| `bridge_check_call` | `src/bridge_run.c` | 1, with a record, if nothing may run: the world has stopped, or a module is pinned to another thread. |
| `bridge_stop` | `src/bridge_stop.c` | Stops the world for the loss of a module that states `fail`. `what` tells what became of the module. |
| `bridge_resume` | `src/bridge_stop.c` | Lets the world run again, if `m` is the module that the world waits for. |
| `bridge_give_up` | `src/bridge_stop.c` | Lets the world run again without the module that stopped it: the host unloads it by name. |
