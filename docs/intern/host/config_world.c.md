# src/host/config_world.c

This file takes the world keys: the phases, the counts, and the budget.

`take_phases` reads `phases` into an array of names.
`take_counts` reads `threads` and `rounds`.
`take_budget` reads the ten budget fields and writes each into `bridge_budget_t` by its offset.
`take_field` copies a four-byte or eight-byte field, and uses `BRIDGE_NONE` when the value is `none`.
`config_world` runs these, then takes the order through `config_order`.

## Functions

| Function | Result |
| --- | --- |
| `config_world` | Fills the world settings of the host. Gives true on success. |
