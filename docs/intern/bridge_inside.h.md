# src/bridge_inside.h

This header gives what the files that make, stop and run a world share, and what the other files of the library need of them.
It includes `src/inside.h`.

`bridge_check_call` takes `repairing`: the module that a load, a reload or an unload names.
A stopped world accepts only a call that names the module that stopped it: `docs/intern/bridge_stop.c.md`, *A stopped world*.

Each `_now` function does the work of its public function, once the call has the world.

## Functions

| Function | Source | Result |
| --- | --- | --- |
| `bridge_open_backends` | `src/bridge_backends.c` | Opens and starts the backends: `docs/intern/bridge_backends.c.md`, *Functions*. |
| `bridge_open_backend` | `src/bridge_backend.c` | Opens and starts one backend: `docs/intern/bridge_backend.c.md`, *Functions*. |
| `bridge_find_phase` | `src/bridge_phases.c` | Finds a phase by its name: `docs/intern/bridge_phases.c.md`, *Functions*. |
| `bridge_name_phases` | `src/bridge_phases.c` | Takes the phases of the deployment: `docs/intern/bridge_phases.c.md`, *Functions*. |
| `bridge_get_stem_length` | `src/bridge_stop.c` | Gives the length of a stem: `docs/intern/bridge_stop.c.md`, *Functions*. |
| `bridge_find_pinned` | `src/bridge_stop.c` | Finds a module pinned to another thread: `docs/intern/bridge_stop.c.md`, *Functions*. |
| `bridge_enter` | `src/bridge_check.c` | Claims the world: `docs/intern/bridge_check.c.md`, *Functions*. |
| `bridge_leave` | `src/bridge_check.c` | Gives the world back: `docs/intern/bridge_check.c.md`, *Functions*. |
| `bridge_check_entry` | `src/bridge_check.c` | Checks the thread of a call: `docs/intern/bridge_check.c.md`, *Functions*. |
| `bridge_check_call` | `src/bridge_check.c` | Checks a call before it runs: `docs/intern/bridge_check.c.md`, *Functions*. |
| `bridge_load_now` | `src/bridge_load.c` | The work of `bridge_load`: `docs/intern/bridge_load.c.md`, *Functions*. |
| `bridge_unload_now` | `src/bridge_unload.c` | The work of `bridge_unload`: `docs/intern/bridge_unload.c.md`, *Functions*. |
| `bridge_reload_now` | `src/bridge_reload.c` | The work of `bridge_reload`: `docs/intern/bridge_reload.c.md`, *Functions*. |
| `bridge_run_phase_now` | `src/bridge_run.c` | The work of `bridge_run_phase`: `docs/intern/bridge_run.c.md`, *Functions*. |
| `bridge_list_modules_now` | `src/bridge_run.c` | The work of `bridge_list_modules`: `docs/intern/bridge_run.c.md`, *Functions*. |
| `bridge_stop` | `src/bridge_stop.c` | Stops the world: `docs/intern/bridge_stop.c.md`, *Functions*. `what` tells what became of the module. |
| `bridge_resume` | `src/bridge_stop.c` | Lets the world run again: `docs/intern/bridge_stop.c.md`, *Functions*. |
| `bridge_give_up` | `src/bridge_stop.c` | Lets the world run without the module: `docs/intern/bridge_stop.c.md`, *A stopped world*. |
