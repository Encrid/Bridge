# src/pool/pool_run.c

This file runs one batch of jobs, shared between the workers of the pool and the caller.

## A run

1. If the batch has fewer than two jobs, or no worker, run the jobs on the caller in order. Then stop.
2. Make the batch the run in progress.
3. Put the index of each job that is not the caller's alone into `order`, in the order of the batch.
4. Set `left` to the count of the jobs. Then set `claim` to the count of `order`, with 0 as the next.
5. Wake one worker for each job after the first, up to the number of workers.
6. Run each job that is the caller's alone on the caller, in the order of the batch.
7. Take and run jobs of `order` on the caller, with the workers.
8. Wait one time for the event `done`.

The first step wakes no thread for a batch that one thread can run.
The release of step 4 gives the batch and `order` to each thread that takes a job: `docs/intern/pool/pool_job.c.md`, *A job*.
The caller runs its own jobs first, and the workers take the other jobs in that time.
So a job of the caller can wait for a job of a worker.

The thread that finishes the last job sets `done` one time in each run, so the caller waits for it one time.
No wait depends on how long a job takes.
After a run, the low half of `claim` is not less than its high half.
So a worker that comes late takes nothing, and no thread reads the arrays of the caller after the run.

## Functions

| Function | Result |
| --- | --- |
| `pool_run_alone` | Runs each job of a batch on the calling thread, in order. |
| `begin_run` | Makes a batch the run in progress with steps 2 to 5 of *A run*. |
| `run_pinned` | Runs each job of the run that is the caller's alone, in order, and counts it as done. |
| `pool_run` | Runs each job of a batch and returns when all ran, with the steps of *A run*. |
