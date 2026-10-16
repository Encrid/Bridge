# src/pool_job.c

This file takes a job of the run in progress, runs it, and counts it as done.
It also gives what the thread of a worker runs.

## A job

A thread takes the first job that no thread took, and marks it taken, under the lock.
A job that is the caller's alone is not taken by a worker.
The thread runs the job, and then counts it as done, under the lock.
The thread that finishes the last job of the run sets the event `done`.
A thread takes jobs until none is left that it may take.

## A worker

A worker sleeps on its event `wake` until it is woken.
Then it takes and runs the jobs that it may take, and sleeps again.
It ends when it wakes and the pool must quit.

## Functions

| Function | Result |
| --- | --- |
| `take_job` | Gives the index of the first job that no thread took, and marks it taken, or -1. Only the caller takes a job that is the caller's alone. |
| `finish_job` | Counts one job as done, and sets the event `done` if it was the last. |
| `pool_run_taken` | Takes jobs of the run in progress and runs them, until none is left to take. `caller` tells if the jobs of the caller may be taken. |
| `pool_work` | What the thread of a worker runs: it sleeps until woken, runs what it can take, and ends when the pool must quit. |
