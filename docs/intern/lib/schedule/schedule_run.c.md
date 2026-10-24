# src/lib/schedule/schedule_run.c

This file runs a phase: a level at a time, one job for each module, and then it counts what failed.
`src/lib/bridge/bridge_run.c` runs a phase of the deployment, and `src/lib/block/block_barrier.c` runs the observers at the barrier, as the phase `phase_count`.

## A level

`run_level` runs one level in these steps:

1. `schedule_gather_jobs` makes the jobs of the level.
2. `pool_run` runs the jobs at the same time, and returns when all of them returned.
3. `account_job` counts each system of each job, and notes each one that gave `BRIDGE_END`.

The count comes after the jobs, on that thread, so the world calls the sink of the host only on that thread.
`schedule_run` runs the levels from 0 to the highest level of the phase.

When `validate` of the world is on, `run_level` runs the jobs one at a time on the caller, with `pool_run_alone`. Then each changed column has one author, which the checksum pass needs: `docs/intern/lib/schedule/schedule_validate.c.md`, *Why a system runs alone*.

## The count of a system

`count_failure` counts the run of one system, by the rules of `docs/intern/lib/schedule/schedule.h.md`, *The failure budget*.
A clean run removes 1 from the count of the system if the count is above 0.
A failed run adds 1, and writes a record at the level `BRIDGE_ERR` with the reason in `why`.
The invocation that failed is abandoned.
The first system of a module that reaches `max_failures` goes in `reached` of the module, and `schedule_account` stops the module at the barrier.

## Functions

| Function | Result |
| --- | --- |
| `count_levels` | Gives how many levels a phase has: one more than its highest level. |
| `count_failure` | Counts what the run of one system came to, and writes a record if it failed. |
| `account_job` | Calls `count_failure` and `schedule_note_end` for each system of a job. |
| `run_level` | Runs the jobs of one level at once, or one at a time when `validate` is on, then counts their failures. |
| `schedule_run` | Runs the systems of one phase, a level at a time. |
