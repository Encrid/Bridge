# src/pool.c

This file makes and unmakes the pool of threads that a phase runs on.

## A pool made

1. Allocate the pool, a byte for each of `most_jobs` jobs, the lock, the event `done` and the workers.
2. If one of them is missing, start no worker.
3. Start the workers one at a time: the event that wakes a worker, then its thread.
4. If a worker does not start, stop.
5. If not every worker started, destroy the pool and give null.

A worker counts only when its thread runs.
So a pool that could not start every worker is unmade a worker at a time, and nothing half made is left.
A pool of 0 workers, or for 0 jobs, still allocates one of each, so that each allocation has a length.

## A pool unmade

`pool_destroy` sets `quit`, and then, for each worker that runs, sets its event, waits for its thread to end and frees its event.
Then it frees the event `done`, the lock, the workers and the bytes of the taken jobs.

## Functions

| Function | Result |
| --- | --- |
| `start_worker` | Starts worker `i`: its event, then its thread. It counts the worker only when the thread runs. 0, or -1 if it cannot start. |
| `pool_create` | Makes a pool of `workers` threads that can run `most_jobs` jobs at one time. Null, with what was made unmade, if there was not the memory. |
| `pool_destroy` | Ends each worker that runs, and frees the pool. A null `p` is nothing to free. |
