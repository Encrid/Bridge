# src/schedule/schedule_jobs.c

This file finds the jobs of a phase, and hands the due systems of one job to the run of its level.

## A run of the jobs

A build that passed made the jobs: `docs/intern/schedule/schedule_commit.c.md`, *The jobs*.
`schedule_find_jobs` finds the first job of a phase by a binary search, since a build sorts the jobs by phase.
`schedule_hand_job` hands one job to the run of its level in these steps:

1. It does not hand a job of a stopped module.
2. It gives each due system of the job its turn and its place, in the order of the job.
3. It puts the job into `run_handed`, and puts 1 into `run_callers` for a pinned module.

A system is always due. An observer is due when something triggered it since the last barrier.
A job at the barrier comes from a triggered observer, so each handed job has a due system.
`schedule_is_due` of `src/schedule/schedule.h` gives this rule to the turns, to the run and to the count, so the 3 always agree.

A parked system is in the list of the world only while `bridge_reload` runs, and no phase runs then.
No build gives a parked system a job.

## The turn of a system

`list_due` gives each due system a number, the ordinal, in the order it hands them in this pass.
It also sets `failed` and `ended` of the system to 0.

| Value | Content |
| --- | --- |
| `turn` | The kind above bit 32, and the ordinal in the low 32 bits. The kind is 0 for an observer, which runs at the barrier, and `TURN_SYSTEM` for a phase. |
| `place` | The ordinal. |

An observer ran at the last barrier, before anything since, and the kind 0 of its turn says so.
The barrier applies the queues in the order of the turns: `docs/intern/storage/storage_runs.h.md`, *The order of the queues*.

## Functions

| Function | Result |
| --- | --- |
| `list_due(pass, job)` | Gives each due system of a job its turn and its place. |
| `schedule_find_jobs(w, phase)` | Gives the index of the first job of the phase, or `job_count` if the phase has no job. |
| `schedule_hand_job(pass, job)` | Hands a job to the run of its level, as *A run of the jobs* tells. |
