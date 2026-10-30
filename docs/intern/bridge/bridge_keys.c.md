# src/bridge/bridge_keys.c

This file refuses each key of a deployment that is unset or out of its range, and keeps the keys in the world.
It runs after the number of the host seam, before the world takes memory for its backends and its phases.

## The keys

| Key | Refused when |
| --- | --- |
| `profile`, `validate` | It is unset, or no item of `enum bridge_flag`. |
| `observer_passes` | It is unset. |
| `table_growth` | It is unset, or below 100. |
| `table_hash_fill`, `table_retry` | It is unset, or 100 or more. |

Each refusal is a record that names the key: `docs/extern/bridge.h.md`, *The deployment*.
`threads` and `stack` are refused where the threads start: `docs/intern/bridge/bridge_create.c.md`, *The threads*.
The world keeps `profile` and `validate` as true when they are `BRIDGE_TRUE`, and the four other keys as they are.

## Functions

| Function | Result |
| --- | --- |
| `refuse_unset` | Refuses a key that is 0, by its name. 1 with a record. |
| `refuse_flag` | Refuses a flag that is unset, or that is neither `BRIDGE_TRUE` nor `BRIDGE_FALSE`. 1 with a record. |
| `refuse_growth` | Refuses a `table_growth` that is unset or below 100. 1 with a record. |
| `refuse_share` | Refuses a percent that is unset, or 100 or more. 1 with a record. |
| `bridge_take_keys` | Refuses the keys of *The keys* in that order, then keeps them in the world. 1 with a record at the first refusal. |
