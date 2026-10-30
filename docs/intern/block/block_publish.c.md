# src/block/block_publish.c

This file publishes the spans of each running module at a barrier, and stops a module whose spans no longer fit.

## A module that no longer fits

The header of a module tells how many table ids there are.
That number is `[budget] tables` and `[budget] sparse_sets`, and 1. It is fixed when the world is made, so one publish is enough.

The barrier stops a module whose span region cannot hold its spans, or the list of an observer: `docs/extern/bridge.h.md`, *The private memory*, and `docs/extern/bridge_abi.h.md`, *observe*.
Its record names the size of its spans, and the observer whose list did not fit.
If its `on_error` is `BRIDGE_FAIL`, the world stops too: `docs/intern/bridge/bridge_stop.c.md`.
If it ran on, its systems would run over tables that they were not told about.
A stopped module keeps its components, and storage writes nothing more for it.
It comes back only as a new record, by a load or a reload, which storage writes as a whole.

## Functions

| Function | Result |
| --- | --- |
| `stop_full` | Stops a module whose spans, or the list of whose observer `list`, no longer fit. Writes its record, and stops the world for `BRIDGE_FAIL`. |
| `block_publish_all` | Publishes the spans of each running module, and stops a module whose spans do not fit. |
