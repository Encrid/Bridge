# src/backends/wasm/wasm_backend.c

This file opens and closes the backend, states its guarantees, and reports the memory shape.
The backend takes no setting, so any setting key is refused.

## What the backend guarantees

`backend_get_capabilities` answers a sandbox with a work limit, a timeout, a memory limit, reload, and side-by-side loading.
It runs a module as wasm in wasmtime, so it can bound work, time and memory, and hold two libraries at once.
`docs/intern/lib/backend/backend.h.md`, *The entries of a backend*, lists what Bridge asks each backend for.

## Starting and stopping

`backend_create` allocates the backend, opens the wake event and the lock, and starts the watchdog thread.
It builds no engine here; `docs/intern/backends/wasm/wasm_adopted.c.md` makes each engine on first need.
`backend_destroy` sets the quit flag, wakes the watchdog, joins it, and deletes every engine.

## Functions

| Function | Result |
| --- | --- |
| `backend_create` | Gives a new backend. Refuses any setting and writes why in `err`. |
| `backend_destroy` | Stops the watchdog, deletes every engine, and frees the backend. |
| `backend_get_capabilities` | Gives the flags the backend keeps: untrusted, work limit, timeout, memory limit, reload, side by side. |
| `backend_get_abi` | Gives `BACKEND_ABI`. Bridge reads it before all else and rejects a mismatch. |
| `memory_get_shape` | Asks for a 4 GiB reservation, a 32 MiB guard, and a 64 KiB granule. |
