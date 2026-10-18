# src/lib/reload/reload_check.c

This file refuses a reload before anything is made, and says that a module is not reloaded.

## What a reload is refused for

A reload is refused for each reason that refuses a load from its config alone: `src/lib/module/module_check.c`.
These refuse a reload too:

- The world has stopped for another module, or a loaded module is pinned to another thread.
- No loaded module has the identity of the artifact, whatever its version.
- The loaded module states `hot_reload` false.
- The artifact names another backend than the backend that runs the module. A reload does not move a module to another backend.

`hot_reload` only forbids. Since there is no flag to reset a module, it gates the one thing that has no other way: a reload in place.
A refusal comes before anything is made, says that the module is not reloaded, and names the reason.
`on_error` does not apply to it: `docs/intern/lib/module/module_say.c.md`, *Refused or not loaded*.

## Functions

| Function | Result |
| --- | --- |
| `reload_refuse` | Says that the loaded module is not reloaded, and why, written as `printf` writes. Gives 1. |
| `reload_check` | Refuses a reload for each reason above, and keeps the backend and the demands of the config, and the loaded module. 1 with a record. |
