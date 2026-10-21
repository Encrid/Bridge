# src/lib/schedule/schedule_build.c

This file builds the schedule: for each phase, and for the barrier as one more phase, the edges, the refusal, then the levels.

## The steps of a phase

`build_phase` schedules one phase in this order:

1. `schedule_gather_phase` lists the systems of the phase: `docs/intern/lib/schedule/schedule_edges.c.md`, *The systems of a phase*.
2. `schedule_add_stated` adds the edges of the pairs of the deployment. They make a closed matrix.
3. `schedule_add_own` adds the edges that each module states in `before`, and keeps the matrix closed.
4. `schedule_check_phase` refuses the phase if it cannot be scheduled.
5. `set_levels` gives each system its level.

Step 3 adds each edge that a module states, also against a pair.

The pairs alone make no cycle, since `check_pairs` refuses such pairs at create: `docs/intern/lib/schedule/schedule_order.c.md`.
`before` alone makes no cycle, since it names only earlier systems.
But a pair against `before` makes a cycle, and step 4 refuses it first: `docs/intern/lib/schedule/schedule_check.c.md`.

## Levels

A level holds each system whose predecessors are all in earlier levels.
`set_levels` puts each system at level 0, then calls `raise_all` until a call raises no system.
A phase that step 4 accepts has no cycle, so the levels stop growing, and the loop ends.
`raise_all` makes each system put every system that it runs before at a level after its own.
A refused phase keeps each system at level 0.

## The loop

`schedule_build` first calls `schedule_cross_phase`, which refuses an `[order]` pair whose two systems a load put in different phases: `docs/intern/lib/schedule/schedule_cross.c.md`.
Then it schedules the entries 0 to `phase_count` of `docs/intern/lib/schedule/schedule.h.md`, *Phases and the barrier*.
First `build_phase` writes in `where` the phase or the barrier, as each reason names it.
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
