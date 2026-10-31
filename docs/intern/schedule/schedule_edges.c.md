# src/schedule/schedule_edges.c

This file gives what orders the systems of a group: `docs/intern/schedule/schedule_group.c.md`.
The pairs of the deployment order them, and so does `before` of the systems of a module.

## The edges

`schedule_add_stated` adds an edge from each system of the group to each one that the pairs put after it: `docs/intern/schedule/schedule_names.c.md`.
The matrix of the pairs is closed, so these edges make a closed matrix, and they need no closure.
A chain of pairs counts also through a system that is out of the phase.

`schedule_add_own` adds the order that a module states.
It adds an edge from system `i` to each other system of its module whose bits of `stated_before` hold the slot of `i`.
It looks in both directions, so the order of `systems` does not matter.
It adds each such edge, also against a pair. `schedule_check_phase` refuses a cycle that this makes: `docs/intern/schedule/schedule_check.c.md`.
After the edges of each system, `order_after` closes the matrix again with `schedule_join_through`.

## Functions

| Function | Result |
| --- | --- |
| `add_stated_from` | Adds an edge from system `i` to each system of the group that the pairs put after it. |
| `schedule_add_stated` | Calls `add_stated_from` for each system of the group. |
| `order_after` | Puts system `i` before each other system of its module whose bits of `stated_before` hold the slot of `i`. Then closes the matrix again with `schedule_join_through`. |
| `schedule_add_own` | Calls `order_after` for each system of the group. |
