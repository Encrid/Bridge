# src/module_check_backend.c

This file refuses a config that asks its backend for what the backend cannot keep.
A stated limit demands that the backend can enforce it.

## Capabilities

A backend states what it can do with its capabilities, a set of flags.
A key of the config of a module demands a capability:

| Key | Capability |
| --- | --- |
| `trusted` is `BRIDGE_FALSE` | `BACKEND_UNTRUSTED` |
| `work_limit` is above 0 | `BACKEND_WORK_LIMIT` |
| `memory_limit` is above 0 | `BACKEND_MEMORY_LIMIT` |
| `init_timeout`, `reload_timeout` or `shutdown_timeout` is above 0 | `BACKEND_TIMEOUT` |
| `hot_reload` is `BRIDGE_TRUE` | `BACKEND_RELOAD` |

A guarantee has one name, for the key that asks for it and for the capability that gives it.
`trusted` is one guarantee: the deployment states whether a module is trusted, and the backend states whether it runs a module that is not.
A key stated as `BRIDGE_NO_LIMIT` demands nothing, so a module on a backend with no limits states its limits that way.

Nothing is downgraded.
A module that asks for a guarantee that its backend cannot give is refused, and never runs without it.
The deployment chose both the config and the backend, so a mismatch is config against config.
It is a refusal of Bridge's own, made before the module exists: `docs/intern/module_say.c.md`, *Refused or not loaded*.

## Reloadable and hot reload

The author states with `reloadable` whether the module can be reloaded.
The deployment states with `hot_reload` whether it may be.
A config that states `hot_reload` and whose author states that the module is not reloadable is refused: it is config against config.

## Functions

| Function | Result |
| --- | --- |
| `check_kept` | Refuses a key that states what the backend cannot keep, by the name of the key and of the backend. |
| `check_limits_kept` | Refuses a limit or `hot_reload` that the backend cannot keep. |
| `module_check_backend` | Refuses a config that states what its backend cannot keep, or that allows what its author says cannot be done. |
