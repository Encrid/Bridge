# src/bridge/bridge_inside.h

This header gives what the files that make, stop and run a world share, and what the other files of the library need of them.
It includes `src/inside.h`.

`bridge_check_call` takes `repairing`: the module that a load, a reload or an unload names.
A stopped world accepts only a call that names the module that stopped it: `docs/intern/bridge/bridge_stop.c.md`, *A stopped world*.

Each `_now` function does the work of its public function, once the call has the world.

## Functions

| Function | Source | Result |
| --- | --- | --- |
| `bridge_take_keys` | `src/bridge/bridge_keys.c` | Refuses and keeps the keys of a deployment: `docs/intern/bridge/bridge_keys.c.md`, *Functions*. |
| `bridge_open_backends` | `src/bridge/bridge_backends.c` | Opens and starts the backends: `docs/intern/bridge/bridge_backends.c.md`, *Functions*. |
| `bridge_open_backend` | `src/bridge/bridge_backend.c` | Opens and starts one backend: `docs/intern/bridge/bridge_backend.c.md`, *Functions*. |
| `bridge_find_phase` | `src/bridge/bridge_phases.c` | Finds a phase by its name: `docs/intern/bridge/bridge_phases.c.md`, *Functions*. |
| `bridge_name_phases` | `src/bridge/bridge_phases.c` | Takes the phases of the deployment: `docs/intern/bridge/bridge_phases.c.md`, *Functions*. |
| `bridge_get_stem_length` | `src/bridge/bridge_stop.c` | Gives the length of a stem: `docs/intern/bridge/bridge_stop.c.md`, *Functions*. |
| `bridge_find_pinned` | `src/bridge/bridge_stop.c` | Finds a module pinned to another thread: `docs/intern/bridge/bridge_stop.c.md`, *Functions*. |
| `bridge_enter` | `src/bridge/bridge_check.c` | Claims the world: `docs/intern/bridge/bridge_check.c.md`, *Functions*. |
| `bridge_leave` | `src/bridge/bridge_check.c` | Gives the world back: `docs/intern/bridge/bridge_check.c.md`, *Functions*. |
| `bridge_check_entry` | `src/bridge/bridge_check.c` | Checks the thread of a call: `docs/intern/bridge/bridge_check.c.md`, *Functions*. |
| `bridge_check_call` | `src/bridge/bridge_check.c` | Checks a call before it runs: `docs/intern/bridge/bridge_check.c.md`, *Functions*. |
| `bridge_load_now` | `src/bridge/bridge_load.c` | The work of `bridge_load`: `docs/intern/bridge/bridge_load.c.md`, *Functions*. |
| `bridge_unload_now` | `src/bridge/bridge_unload.c` | The work of `bridge_unload`: `docs/intern/bridge/bridge_unload.c.md`, *Functions*. |
| `bridge_reload_now` | `src/bridge/bridge_reload.c` | The work of `bridge_reload`: `docs/intern/bridge/bridge_reload.c.md`, *Functions*. |
| `bridge_run_phase_now` | `src/bridge/bridge_run.c` | The work of `bridge_run_phase`: `docs/intern/bridge/bridge_run.c.md`, *Functions*. |
| `bridge_list_modules_now` | `src/bridge/bridge_run.c` | The work of `bridge_list_modules`: `docs/intern/bridge/bridge_run.c.md`, *Functions*. |
| `bridge_stop` | `src/bridge/bridge_stop.c` | Stops the world: `docs/intern/bridge/bridge_stop.c.md`, *Functions*. `what` tells what became of the module. |
| `bridge_resume` | `src/bridge/bridge_stop.c` | Lets the world run again: `docs/intern/bridge/bridge_stop.c.md`, *Functions*. |
| `bridge_give_up` | `src/bridge/bridge_stop.c` | Lets the world run without the module: `docs/intern/bridge/bridge_stop.c.md`, *A stopped world*. |
