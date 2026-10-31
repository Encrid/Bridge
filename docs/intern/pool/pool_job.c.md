# src/pool/pool_job.c

This file takes a job of the run in progress, runs it, and counts it as done.
It also gives what the thread of a worker runs.

## A job

1. A thread adds 1 to `claim` in one atomic step, and reads the value from before the step.
2. If the low half of that value is less than its high half, the thread takes that place of `order`.
3. Else no job is left, and the thread stops.
4. The thread runs the job, and then takes 1 from `left` in one atomic step.
5. The thread that finishes the last job of the run sets the event `done`.

Each step of `claim` gives a different low half, so each place of `order` goes to one thread.
A thread that comes after the end of a run gets a low half that is not less than the count, so it takes nothing.
If the next run started before its step, it takes a job of that run, with the batch of that run.
The step of `claim` acquires what the caller wrote before the run, so the thread reads the batch and `order` of its run.
The step of `left` releases what the job wrote. The last step acquires each of them, before the event `done`.

## A worker

A worker sleeps on its event `wake` until it is woken.
Then it takes and runs jobs until none is left, and sleeps again.
It ends when it wakes and the pool must quit.

## Functions

| Function | Result |
| --- | --- |
| `take_job` | Gives the index in the batch of the next job of `order`, or -1 if none is left. |
| `pool_finish_job` | Counts one job as done, and sets the event `done` if it was the last. |
| `pool_run_taken` | Takes jobs of `order` and runs them, until none is left to take. |
| `pool_work` | What the thread of a worker runs: it sleeps until woken, runs what it can take, and ends when the pool must quit. |
