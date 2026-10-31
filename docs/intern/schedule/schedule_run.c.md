# src/schedule/schedule_run.c

This file runs a phase: a level at a time, one job for each module, and then it counts what failed.
`src/bridge/bridge_run.c` runs a phase of the deployment, and `src/block/block_barrier.c` runs the observers at the barrier, as the phase `phase_count`.

## A level

`run_jobs` runs the jobs of a phase from `schedule_find_jobs`, a level at a time, until a job of another phase:

1. `schedule_hand_job` hands each job of the level: `docs/intern/schedule/schedule_jobs.c.md`, *A run of the jobs*.
2. `schedule_run_batch` runs the handed jobs at the same time with `pool_run`, and returns when all of them returned.
3. Then `account_job` notes the module of each handed job on the work list. It counts each due system of the job, and notes each one that gave `BRIDGE_END`.

The count comes after the jobs, on that thread, so the world calls the sink of the host only on that thread.
The barrier runs only the jobs of the observers that something triggered: `docs/intern/schedule/schedule_triggered.c.md`, *A run at the barrier*.

When `validate` of the world is on, `schedule_run_batch` runs the jobs one at a time on the caller, with `pool_run_alone`. Then each changed column has one author, which the checksum pass needs: `docs/intern/schedule/schedule_validate.c.md`, *Why a system runs alone*.

## The count of a system

`count_failure` counts the run of one system, by the rules of `docs/intern/schedule/schedule.h.md`, *The failure budget*.
A clean run removes 1 from the count of the system if the count is above 0.
A failed run adds 1, and writes a record at the level `BRIDGE_ERR` with the reason in `why`.
The invocation that failed is abandoned.
The first system of a module that reaches `max_failures` goes in `reached` of the module, and `schedule_account` stops the module at the barrier.

## Functions

| Function | Result |
| --- | --- |
| `count_failure(w, s)` | Counts what the run of one system came to, and writes a record if it failed. |
| `account_job(w, job)` | Notes the module of a job on the work list: `docs/intern/block/block_work.c.md`. Calls `count_failure` and `schedule_note_end` for each due system of the job. |
| `run_jobs(pass, first)` | Runs the jobs of a phase from the job `first`, a level at a time, as *A level* tells. |
| `schedule_run_batch(pass)` | Runs the handed jobs of a level at once, or one at a time when `validate` is on, then counts them. |
| `schedule_run(w, phase)` | Runs the jobs of one phase, or the jobs of the triggered observers at the barrier. |
