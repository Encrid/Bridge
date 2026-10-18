# src/lib/bridge/bridge_inside.h

This header gives what the files that make, stop and run a world share, and what the other files of the library need of them.
It includes `src/lib/inside.h`.

`bridge_check_call` takes `repairing`: the module that a load, a reload or an unload names.
A stopped world accepts only a call that names the module that stopped it: `docs/intern/lib/bridge/bridge_stop.c.md`, *A stopped world*.

Each `_now` function does the work of its public function, once the call has the world.

## Functions

| Function | Source | Result |
| --- | --- | --- |
| `bridge_open_backends` | `src/lib/bridge/bridge_backends.c` | Opens and starts the backends: `docs/intern/lib/bridge/bridge_backends.c.md`, *Functions*. |
| `bridge_open_backend` | `src/lib/bridge/bridge_backend.c` | Opens and starts one backend: `docs/intern/lib/bridge/bridge_backend.c.md`, *Functions*. |
| `bridge_find_phase` | `src/lib/bridge/bridge_phases.c` | Finds a phase by its name: `docs/intern/lib/bridge/bridge_phases.c.md`, *Functions*. |
| `bridge_name_phases` | `src/lib/bridge/bridge_phases.c` | Takes the phases of the deployment: `docs/intern/lib/bridge/bridge_phases.c.md`, *Functions*. |
| `bridge_get_stem_length` | `src/lib/bridge/bridge_stop.c` | Gives the length of a stem: `docs/intern/lib/bridge/bridge_stop.c.md`, *Functions*. |
| `bridge_find_pinned` | `src/lib/bridge/bridge_stop.c` | Finds a module pinned to another thread: `docs/intern/lib/bridge/bridge_stop.c.md`, *Functions*. |
| `bridge_enter` | `src/lib/bridge/bridge_check.c` | Claims the world: `docs/intern/lib/bridge/bridge_check.c.md`, *Functions*. |
| `bridge_leave` | `src/lib/bridge/bridge_check.c` | Gives the world back: `docs/intern/lib/bridge/bridge_check.c.md`, *Functions*. |
| `bridge_check_entry` | `src/lib/bridge/bridge_check.c` | Checks the thread of a call: `docs/intern/lib/bridge/bridge_check.c.md`, *Functions*. |
| `bridge_check_call` | `src/lib/bridge/bridge_check.c` | Checks a call before it runs: `docs/intern/lib/bridge/bridge_check.c.md`, *Functions*. |
| `bridge_load_now` | `src/lib/bridge/bridge_load.c` | The work of `bridge_load`: `docs/intern/lib/bridge/bridge_load.c.md`, *Functions*. |
| `bridge_unload_now` | `src/lib/bridge/bridge_unload.c` | The work of `bridge_unload`: `docs/intern/lib/bridge/bridge_unload.c.md`, *Functions*. |
| `bridge_reload_now` | `src/lib/bridge/bridge_reload.c` | The work of `bridge_reload`: `docs/intern/lib/bridge/bridge_reload.c.md`, *Functions*. |
| `bridge_run_phase_now` | `src/lib/bridge/bridge_run.c` | The work of `bridge_run_phase`: `docs/intern/lib/bridge/bridge_run.c.md`, *Functions*. |
| `bridge_list_modules_now` | `src/lib/bridge/bridge_run.c` | The work of `bridge_list_modules`: `docs/intern/lib/bridge/bridge_run.c.md`, *Functions*. |
| `bridge_stop` | `src/lib/bridge/bridge_stop.c` | Stops the world: `docs/intern/lib/bridge/bridge_stop.c.md`, *Functions*. `what` tells what became of the module. |
| `bridge_resume` | `src/lib/bridge/bridge_stop.c` | Lets the world run again: `docs/intern/lib/bridge/bridge_stop.c.md`, *Functions*. |
| `bridge_give_up` | `src/lib/bridge/bridge_stop.c` | Lets the world run without the module: `docs/intern/lib/bridge/bridge_stop.c.md`, *A stopped world*. |
