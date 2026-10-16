# src/schedule_names.c

This file reads the names of `[order]` that the world keeps: `docs/intern/schedule_order.c.md`.

## The index of a system

Each system has `named`: the index of its full name among the names of the pairs.
A system that no pair names has the count of names.
`src/abi_subscriber_room.c` sets `named` once, when the module registers the system.
Neither the full name of a system nor the pairs change after that.

## What orders two systems

The pairs put system `s` before system `t` when the closed matrix of the names puts the name of `s` before the name of `t`.
The matrix is closed at create, so a chain of pairs counts also through the name of a system that is not loaded.

## Functions

| Function | Result |
| --- | --- |
| `schedule_find_name` | Gives the index of a full name among the names of the pairs, or the count of names if no pair names it. |
| `schedule_orders` | Gives whether the pairs put system `s` before system `t`. |
