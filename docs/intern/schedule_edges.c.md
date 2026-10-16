# src/schedule_edges.c

This file gives the systems of one phase and what orders them.
The pairs of the deployment order them, and so does the order in which a module registered its own systems.

## The systems of a phase

`schedule_gather_phase` lists each system of the phase that is not parked, in the order of `systems` of the world, which is the order of registration.
A parked system is out of the schedule while a reload decides what becomes of it.
The list is in the room of `build_in`, and the matrix is in the room of `build_bits`, which the function makes zero.
That room is `[budget] systems` wide, and `schedule_room` makes it at create.

## The edges

`schedule_add_stated` adds an edge for each pair of `[order]` whose two systems are in the phase.
It finds a system by its full name: the identity of its module without the version, a colon, and its own name.
A pair that names a system out of the phase adds nothing.

`schedule_add_own` adds the order of a module: each system runs before each later system of its module that it conflicts with.
It adds no edge from a system to one that already runs before it, so a pair of the deployment stands against it.

## Functions

| Function | Result |
| --- | --- |
| `schedule_gather_phase` | Lists the systems of the phase that are not parked, with an empty matrix for them. |
| `find_system` | Gives the index of the system of the phase that has a full name, or how many systems there are if none has it. |
| `schedule_add_stated` | Adds the edges that the pairs of the deployment state between systems of the phase. |
| `order_after` | Puts system `i` before each later system of its module that it conflicts with, unless that one already runs before it. |
| `schedule_add_own` | Calls `order_after` for each system of the phase. |
