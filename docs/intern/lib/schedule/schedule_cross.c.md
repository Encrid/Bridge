# src/lib/schedule/schedule_cross.c

This file refuses an `[order]` pair whose two systems run in different phases.
`schedule_build` calls `schedule_cross_phase` before it schedules the phases: `docs/intern/lib/schedule/schedule_build.c.md`, *The loop*.

## A pair of two phases

A pair orders one system before another within a phase: a phase runs whole before the next, so an order across two phases orders nothing.
`schedule_order` keeps each pair by its two indices into the names: `docs/intern/lib/schedule/schedule_order.c.md`, *The pairs*.
`system_of` gives the last system of a name.
A reload adds its new system after the old one, so the last is the one it loaded.
If both systems of a pair are loaded and their phases differ, the load is refused, and `why` names the two systems.

A pair whose names no system has, or whose two systems a load did not both load, orders nothing to refuse.
The barrier is one phase after the phases of the deployment.
So a pair of a system and an observer is a pair of two phases.

## Functions

| Function | Result |
| --- | --- |
| `system_of` | Gives the last loaded system of a name, by its index among the names, or `nullptr`. |
| `schedule_cross_phase` | Refuses a pair whose two systems run in different phases. Gives 1 with `why`, or 0. |
