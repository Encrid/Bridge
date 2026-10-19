# src/host/host_load.c

This file loads a found module into the world.

## The load

`load_found` runs the gates, asks to trust an arriving module, then calls `bridge_load`.
`is_left` turns a module down when a peer already holds its name, when its ABI differs, or when the host lacks its backend.
`is_stated` reads `content.load` and builds the artifact when the module is to load.
`make_artifact` fills a `bridge_artifact_t` with the bytes or the path and the module config.
`get_same` finds a loaded module of the same name.

## Functions

| Function | Result |
| --- | --- |
| `load_found` | Loads a module and prints the outcome. |
| `make_artifact` | Builds the artifact of a module. Gives true when its keys are taken. |
