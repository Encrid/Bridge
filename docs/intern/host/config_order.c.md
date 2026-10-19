# src/host/config_order.c

This file takes the `order.before` pairs for Bridge.

`config_order` reads each pair stated in any file.
`add_order` keeps a pair once, and uses `config_find` to catch a clash between peers.
`states_pair` finds a given pair inside the `order.before` list of a file.
`is_pair` tests whether two names form a pair, in either order.

## Functions

| Function | Result |
| --- | --- |
| `config_order` | Fills the order array. Gives true when every pair agrees. |
