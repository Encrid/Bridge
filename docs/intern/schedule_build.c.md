# src/schedule_build.c

This file builds the schedule: for each phase, and for the barrier as one more phase, the edges, the refusal, then the levels.

## The steps of a phase

`build_phase` schedules one phase in this order:

1. `schedule_gather_phase` lists the systems of the phase that are not parked.
2. `schedule_add_stated` adds the edges of the pairs of the deployment. They make a closed matrix.
3. `schedule_add_own` adds the edges that each module states by its order of registration, and keeps the matrix closed.
4. `schedule_check_phase` refuses the phase if it cannot be scheduled.
5. `set_levels` gives each system its level.

Step 3 adds no edge from a system to one that already runs before it, also through a chain of edges.
So a pair of the deployment stands against the order of a module, as `docs/extern/bridge.h.md` says.

No phase has a cycle: no system of a phase runs before itself.
The pairs of the deployment have no cycle, since `check_pairs` refuses such pairs at create: `docs/intern/schedule_order.c.md`.
Step 3 adds no cycle, since it adds no edge to a system that already runs before the first one.

## Levels

A level holds each system whose predecessors are all in earlier levels.
`set_levels` puts each system at level 0, then calls `raise_all` until a call raises no system.
A phase has no cycle, so the levels stop growing, and the loop ends.
`raise_all` makes each system put every system that it runs before at a level after its own.
A refused phase keeps each system at level 0.

## The loop

`schedule_build` schedules the phases 0 to `phase_count`, where `phase_count` is the barrier.
All of them use the room of `build_in` and `build_bits`, and only the levels stay in the systems.
The loop stops at the first refused phase, and `why` then says why.

## Functions

| Function | Result |
| --- | --- |
| `raise_after` | Puts each system that system `i` runs before at a level after the level of `i`. Gives whether it raised one. |
| `raise_all` | Calls `raise_after` for each system of the phase. Gives whether one call raised a system. |
| `set_levels` | Gives each system its level. A refused phase has each system at level 0. |
| `build_phase` | Schedules one phase. Gives 1 if the phase is refused. |
| `schedule_build` | Schedules every phase and the barrier. Gives 0, or 1 with `why`. |
