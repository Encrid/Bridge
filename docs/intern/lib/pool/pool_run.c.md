# src/lib/pool/pool_run.c

This file runs one batch of jobs, shared between the workers of the pool and the caller.

## A run

1. If the batch has fewer than two jobs, or no worker, run the jobs on the caller in order. Then stop.
2. Make the batch the run in progress, with no job taken, under the lock.
3. Wake one worker for each job after the first, up to the number of workers.
4. Take and run jobs on the caller, the jobs of the caller with them.
5. Wait until each job of the run is done.
6. Leave no job to take, under the lock.

The first step wakes no thread for a batch that one thread can run.
The last step is necessary because the arrays of the batch are the caller's.
Nothing of a run may be taken after the caller's arrays are gone.

## Functions

| Function | Result |
| --- | --- |
| `run_alone` | Runs each job of a batch on the calling thread, in order. |
| `begin_run` | Makes a batch the run in progress, with no job taken, and wakes a worker for each job after the first. |
| `end_run` | Waits until each job of the run is done, and then leaves no job to take. |
| `pool_run` | Runs each job of a batch and returns when all ran, with the steps of *A run*. |
