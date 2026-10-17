# src/bridge_calls.c

This file gives the functions of the world that a host calls, but `bridge_create` and `bridge_destroy`.
Each one claims the world for its thread, calls the function that does the work, and gives the world back.
A call from another thread while a call of the world runs does nothing: `docs/intern/bridge_check.c.md`, *A call from another thread*.

## Functions

| Function | Result |
| --- | --- |
| `bridge_load` | `bridge_load_now`, or 1 if another thread has the world. |
| `bridge_unload` | `bridge_unload_now`, or 1 if another thread has the world. |
| `bridge_reload` | `bridge_reload_now`, or 1 if another thread has the world. |
| `bridge_run_phase` | `bridge_run_phase_now`, or 1 if another thread has the world. |
| `bridge_list_modules` | `bridge_list_modules_now`, or -1 if another thread has the world. |
