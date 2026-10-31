# src/schedule/schedule_commit.c

This file makes the jobs of each phase and of the barrier, at the end of a build that passed.

## The jobs

For each phase, then for the barrier, `schedule_commit_jobs` does these steps:

1. `gather_phase` collects the systems of the phase, and leaves out the parked ones. They keep their order.
2. `count_levels` counts the systems of each level, and `place_levels` writes them into `run_lists` by level. A level keeps the order of the list.
3. `make_jobs` makes a job of each run of systems of one module in one level. It writes the index of each job into `job` of its systems.

A module puts its systems in one run of the list of the world: `docs/intern/abi/abi_subscribe.c.md`, *The slot*.
So a module has one job in each level where it has a system.
So the jobs go by phase, then by level, then by the place of each module in the list of the world.
`run_jobs` holds them, and `job_count` of the world gives their count.

The build that passed no longer needs `build_in` and `build_bits`. So `gather_phase` lists into `build_in`, and `count_levels` counts in `build_bits`.
A phase has at most as many levels as systems, and `build_bits` has more words than `[budget] systems`.

## Functions

| Function | Result |
| --- | --- |
| `gather_phase(w, phase)` | Collects the systems of the phase into `build_in`, with no parked one, and gives their count. |
| `count_levels(w, n)` | Counts the systems of each level of the list in `build_bits`, and gives the count of levels. |
| `place_levels(w, n, at, levels)` | Writes the list into `run_lists` from `at`, by level, in the order of the list in each level. |
| `make_jobs(w, at, n)` | Makes a job of each run of systems of one module and one level, as *The jobs* tells. |
| `schedule_commit_jobs(w)` | Makes the jobs of each phase and of the barrier, as *The jobs* tells. |
