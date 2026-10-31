# src/schedule/schedule_names.c

This file reads the names of `[order]` that the world keeps: `docs/intern/schedule/schedule_order.c.md`.

## The index of a system

Each system has `named`: the index of its full name in the list of the names of the pairs.
A system that no pair names has the count of names.
`src/abi/abi_subscriber_room.c` sets `named` first, when the module registers the system.
`schedule_mark_all` calls `schedule_resolve_names` before the build of a new `[order]`, since `bridge_set_order` changes the pairs: `docs/intern/bridge/bridge_order.c.md`.
The full name of a system does not change, so its index follows only the pairs.

## What orders two systems

The pairs put system `s` before system `t` when the closed matrix of the names puts the name of `s` before the name of `t`.
The matrix is closed when the order is built, so a chain of pairs counts through the name of a system that is not loaded.

## Functions

| Function | Result |
| --- | --- |
| `schedule_find_name` | Gives the index of a full name among the names of the pairs, or the count of names if no pair names it. |
| `schedule_orders` | Gives whether the pairs put system `s` before system `t`. |
| `schedule_resolve_names` | Sets the `named` index of each system again from its full name, against the current pairs. |
| `schedule_free_pairs` | Frees the names, the pairs, and the matrix of an order set. |
