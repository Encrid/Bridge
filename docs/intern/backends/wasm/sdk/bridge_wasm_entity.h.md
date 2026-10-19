# src/backends/wasm/sdk/bridge_wasm_entity.h

This header reads entities and finds a component value by entity id.
It reaches the entity index and the column bases through the private header.

## Functions

| Function | Result |
| --- | --- |
| `bridge_get_entity` | Gives the id in a row of a span's entity column. |
| `bridge_get_ref` | Gives the address of an entity's value for a slot, or zero when the id is stale or absent. |
| `bridge_is_alive` | Gives whether an id still names a live entity. |
