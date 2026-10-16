# src/schedule_edges.c

This file gives the systems of one phase and what orders them.
The pairs of the deployment order them, and so does the order in which a module registered its own systems.

## The systems of a phase

`schedule_gather_phase` lists each system of the phase that is not parked, in the order of `systems` of the world, which is the order of registration.
A parked system is out of the schedule while a reload decides what becomes of it.
The list is in the room of `build_in`, and the matrix is in the room of `build_bits`, which the function makes zero.
That room is `[budget] systems` wide, and `schedule_room` makes it at create.

## The edges

`schedule_add_stated` adds an edge from each system of the phase to each one that the pairs put after it: `docs/intern/schedule_names.c.md`.
The matrix of the pairs is closed, so these edges make a closed matrix, and they need no closure.
A chain of pairs counts also through a system that is out of the phase.

`schedule_add_own` adds the order of a module: each system runs before each later system of its module that it conflicts with.
It adds no edge from a system to one that already runs before it, so a pair of the deployment stands against it.
After the edges of each system, `order_after` closes the matrix again with `schedule_join_through`.
So the test of a later system sees each chain of the edges before it.

## Functions

| Function | Result |
| --- | --- |
| `schedule_gather_phase` | Lists the systems of the phase that are not parked, with an empty matrix for them. |
| `add_stated_from` | Adds an edge from system `i` to each system of the phase that the pairs put after it. |
| `schedule_add_stated` | Calls `add_stated_from` for each system of the phase. |
| `order_after` | Puts system `i` before each later system of its module that it conflicts with, unless that one already runs before it. Then closes the matrix again with `schedule_join_through`. |
| `schedule_add_own` | Calls `order_after` for each system of the phase. |
