# src/schedule/schedule_build.c

This file builds the schedule after a change of the systems: first the checks of all the entries, then the levels.
It looks only at the groups of the marked systems: `docs/intern/schedule/schedule_marks.c.md`.

## The loop

1. Write in `at` of each system its place in `systems` of the world.
2. Refuse a pair of `[order]` that names a fresh system, if its systems run in 2 phases.
3. Check the entries 0 to `phase_count` in order, and stop at the first refused entry.
4. If step 2 or step 3 refused, give 1 with `why`, and stop.
5. Give the levels of the group of each entry.
6. Take away the marks of each system.
7. Make the jobs with `schedule_commit_jobs`: `docs/intern/schedule/schedule_commit.c.md`.

Step 2 is `schedule_cross_phase`: `docs/intern/schedule/schedule_cross.c.md`.
The entries are those of `docs/intern/schedule/schedule.h.md`, *Phases and the barrier*.
A refused build changes no level, no mark and no job, so the jobs stay those of the last build that passed.

## The check of an entry

`check_phases` first writes in `where` the phase or the barrier, as each reason names it.
Then it checks the entry in this order:

1. `schedule_gather_group` gives the group of the entry: `docs/intern/schedule/schedule_group.c.md`.
2. `schedule_add_stated` adds the edges of the pairs. They make a closed matrix.
3. `schedule_add_own` adds the edges that each module states in `before`, and keeps the matrix closed.
4. `schedule_check_phase` refuses the entry if its group cannot be scheduled.

Step 3 adds each edge that a module states, also against a pair.
The pairs alone make no cycle, since `check_pairs` refuses such pairs at create: `docs/intern/schedule/schedule_order.c.md`.
`before` alone makes no cycle, since it names only earlier systems.
But a pair against `before` makes a cycle, and step 4 refuses it first: `docs/intern/schedule/schedule_check.c.md`.

## The levels

`set_phase_levels` makes the group of each entry again, with steps 1 to 3 of *The check of an entry*.
Then `schedule_set_levels` sets the levels of the group: `docs/intern/schedule/schedule_levels.c.md`.
All the steps use the room of `build_in` and `build_bits`, and only the levels stay in the systems.

## Functions

| Function | Result |
| --- | --- |
| `number_systems` | Writes in `at` of each system its place in `systems`. |
| `make_group` | Gives the group of an entry and its closed matrix, with steps 1 to 3 of *The check of an entry*. |
| `check_phases` | Checks the group of each entry, in order. Gives 1 at the first refused entry, with `why`. |
| `set_phase_levels` | Gives the levels of the group of each entry. |
| `schedule_build` | Builds the schedule with the steps of *The loop*. Gives 0, or 1 with `why`. |
