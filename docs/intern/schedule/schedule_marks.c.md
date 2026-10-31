# src/schedule/schedule_marks.c

This file marks the systems that the next build must look at.
A build looks only at the groups of these systems: `docs/intern/schedule/schedule_group.c.md`, *A group*.

## The marks

| Mark | Meaning | Set by |
| --- | --- | --- |
| `fresh` | The system came after the last build that passed. | `list_system`, when a module registers the system: `docs/intern/abi/abi_subscribe.c.md`. |
| `dirty` | The levels of the group of the system can change. | A fresh system, a system that comes back from a park, and each neighbor of a system that goes. |

Only a fresh system can make a conflict that nothing orders, a cycle, or a pair of 2 phases.
An unload, a park and a failed load only take systems away, so they make none: `docs/intern/schedule/schedule.h.md`, *What orders two systems*.
A build that passes takes away both marks of each system: `docs/intern/schedule/schedule_build.c.md`, *The loop*.
A refused build keeps them, so the next build looks at the same systems again.

## Neighbors

Two systems are neighbors when they are in one phase, and an edge joins them in either direction:

- a pair of `[order]`, through the closed matrix of the names: `docs/intern/schedule/schedule_names.c.md`;
- `before` of their module, when both are of one module: `docs/intern/schedule/schedule_before.c.md`.

A parked system is no neighbor, since it is out of the schedule while its module reloads.
A system that goes can lower the levels of its neighbors.
So `abi_park` and `schedule_leave_module` mark each neighbor of such a system dirty.

## The whole schedule

`bridge_set_order` changes the pairs, so the new pairs can order or unorder any 2 systems.
Before its build, `schedule_mark_all` finds the index of each system among the new names, and marks each system fresh and dirty.
So that build checks each pair of systems of each phase, and gives each system its level.

## Functions

| Function | Result |
| --- | --- |
| `schedule_is_neighbor` | Gives whether `t` is a neighbor of `s`: in the phase of `s`, not parked, not `s`, and an edge joins them. |
| `schedule_mark_neighbors` | Marks each neighbor of `s` dirty. |
| `schedule_leave_module` | Marks the neighbors of each system of a module dirty, and takes each one out of the index of the components. |
| `schedule_mark_all` | Finds the index of each system among the names of `[order]` again, and marks each system fresh and dirty. |
