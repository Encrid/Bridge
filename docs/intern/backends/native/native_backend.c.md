# src/backends/native/native_backend.c

This file starts and stops the backend, states what it guarantees, and adopts a block of memory.
The backend has no setting of its own.

## What the backend guarantees

`backend_get_capabilities` answers `BACKEND_RELOAD | BACKEND_UNLOAD`.
It keeps no work limit, no timeout, and no cap on memory, because a module runs as code in this process.
It also cannot hold two copies of one library at once.

## Functions

| Function | Result |
| --- | --- |
| `backend_create` | Gives a new backend. Any setting key is unknown, so it writes why in `err` and refuses. |
| `backend_destroy` | Frees the backend. |
| `backend_get_capabilities` | Gives the flags the backend keeps: reload and unload. |
| `memory_get_shape` | Asks for no reservation, no guard and no granule. |
| `memory_adopt` | Wraps a block Bridge reserved, which is already addressable. |
