# src/backends/wasm/wasm_inside.h

The wasm backend's files share this private header.
It pulls in wasmtime, sets the memory constants, numbers the engine kinds, and declares the backend structs and the shared helpers.

## The constants and the engine kinds

`WASM_RESERVATION` is 4 GiB, `WASM_GUARD` is 32 MiB, and `WASM_PAGE` is 64 KiB: the shape every engine and memory holds to.
`WASM_FAR` is an epoch deadline that no call reaches until the watchdog arms a nearer one.
`enum wasm_engine` carries two bits, metered and interruptible, so a module needs one of four engines.

## The structs

`backend` holds the four engines, the watchdog, the wake event, the lock, the armed call, and the block being adopted.
`backend_memory`, `backend_module`, `backend_instance` and `backend_func` hold an adopted block, a compiled module, a live instance, and a kept function.
`wasm_own_t` and `wasm_offered_t` hold a module's own memory and a wrapped host function.

## Functions

| Function | Result |
| --- | --- |
| `wasm_drop` and `wasm_fail` | Declared here, given by `docs/intern/backends/wasm/wasm_fail.c.md`. |
| `wasm_make_own` | Declared here, given by `docs/intern/backends/wasm/wasm_own.c.md`. |
| `wasm_adopt_into` | Declared here, given by `docs/intern/backends/wasm/wasm_holder.c.md`. |
| `wasm_watch` | Declared here, given by `docs/intern/backends/wasm/wasm_engine.c.md`. |
| `wasm_get_engine` | Declared here, given by `docs/intern/backends/wasm/wasm_adopted.c.md`. |
| `wasm_make_offered`, `wasm_make_types` and `wasm_call_offered` | Declared here, given by `docs/intern/backends/wasm/wasm_offered.c.md`. |
