# src/schedule_jobs.c

This file makes the jobs of one level of a phase: for each module, its systems that run in the level, in one list.

## The jobs of a level

`schedule_gather_jobs` looks at the systems of the world in the order of registration.
At the first system of a module that has no job yet, `fill_job` makes the next job of the level for that module.
The job lists each system of the module, from that one on, that runs now.
A job with no system is not counted.
The list of each job is one run of `run_lists`, after the list of the job before it.

A system runs now if all these are true:

- It is of this phase and of this level.
- Its module is not stopped.
- It is a system, or it is an observer that something triggered since the last barrier.

A parked system is in the list of the world only while `bridge_reload` runs, and no phase runs then.
A call from the sink is refused, and a call from another thread may not overlap: `docs/extern/bridge.h.md`.
So `runs_now` does not test `parked`.

`run_handed` gets a pointer to each job, for `pool_run`.
`run_callers` gets 1 for the job of a pinned module: the thread that asked for the phase runs that job.

## The turn of a system

`list_system` gives each system that runs a number, the ordinal, in the order it lists them in this pass.
It also sets `failed` of the system to 0.

| Value | Content |
| --- | --- |
| `turn` | The kind above bit 32, and the ordinal in the low 32 bits. The kind is 0 for an observer, which runs at the barrier, and `TURN_SYSTEM` for a phase. |
| `place` | The ordinal. |

An observer ran at the last barrier, before anything since, and the kind 0 of its turn says so.
The barrier applies the queues in the order of the turns: `docs/intern/storage_runs.h.md`, *The order of the queues*.

## Functions

| Function | Result |
| --- | --- |
| `has_job` | Gives whether a module has a job in the level already. |
| `runs_now` | Gives whether a system runs in this level of this phase. |
| `list_system` | Adds a system to a job and gives it its turn and its place. |
| `fill_job` | Makes the next job of the level for the module of system `i`, with each of its systems from `i` on that runs now. |
| `schedule_gather_jobs` | Makes the jobs of one level of a phase, one for each module that has a system that runs in it. |
