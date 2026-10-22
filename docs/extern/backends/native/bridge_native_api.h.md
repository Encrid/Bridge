# include/backends/native/bridge_native_api.h

This header gives the table the backend hands a module, and the scratch arena the entries use.
`bridge_api_t` holds the eight entry pointers and the start of each of the two memories.

## The table and the arena

`BRIDGE_NATIVE` declares a module's exported function, named with a `bridge_` prefix and handed the table.
`BRIDGE_NATIVE_STATE` places a struct of a given type into the arena.
Every pointer argument of an entry is an offset into that arena, which Bridge reads and bounds-checks.

## The entries

`bridge_api` holds ten pointers: eight entries and two memory bases.
The entries are `spawn` and `destroy`, `attach` and `detach`, `declare`, `add_system`, `observe` and `alloc`.
The bases are `shared_base`, the shared block, and `private_base`, the module's own.

## Functions

| Function | Result |
| --- | --- |
| `bridge_native_get_header` | Gives the header at the start of the private memory. |
| `bridge_native_put` | Copies bytes into the arena and gives their offset, or zero when they do not fit. |
| `bridge_native_lend` | Copies bytes above the used arena without counting them, for a value an entry copies during the call. |
