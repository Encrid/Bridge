# src/pool.h

This header gives the pool of threads that a phase runs on: `threads - 1` workers kept for the life of the deck, and the caller.
Nothing in the pool knows what a job is.

## The execution model

The systems of different modules run at the same time, each module on a thread, when they do not conflict.
The systems of one module run one after another, each from its start to its return.
Two systems of one module never run at the same time, and one never waits paused for another.
The schedule finds the order a system at a time, and what runs at the same time a module at a time: `src/schedule.h`.

The systems of one module cannot overlap, because they share what only one of them may use.
On the wasm backend, a module is one instance in one store, and one thread at a time enters a store.
On each backend, a module has one private memory.
Before each call, the library writes there which system runs, and the module moves one scratch pointer and one log ring head there.
On the native backend, a module also has its own globals, and nothing guards them.

So one heavy system cannot use more than one core.
A deployment gets parallel work from more modules whose systems do not conflict, and not from more systems.
One loop over one component cannot be split between two modules either: both would write it, conflict, and be put in order.

`threads` states how many threads run a phase, and it counts the thread that called `bridge_run_phase`.
That thread works too, and at `threads = 1` it does all the work, in an order that never changes.
The pool keeps the other threads for as long as the deck exists.

## Types

| Type | Content |
| --- | --- |
| `pool_fn_t` | The function that a job is given to. |
| `pool_batch_t` | What one run is handed: `count` jobs in `jobs`, the function `fn` that each is given to, and `caller_only`. |
| `pool_worker_t` | A worker: its pool `p`, the event `wake` that wakes it, and its `thread`. |
| `pool_t` | The pool, `struct pool`: its `workers` and `worker_count`, its `lock`, its event `done`, `quit`, and the run in progress. |

`caller_only` holds a byte for each job of a batch: 1 if the job is the caller's alone.
The run in progress is `run`, `taken` and `left`: the batch, a byte for each job that a thread took, and the jobs not done.
Each thread reads and writes the run in progress only under the lock.

## Functions

| Function | Source | Result |
| --- | --- | --- |
| `pool_create` | `src/pool.c` | Makes a pool of `workers` threads that can run `most_jobs` jobs at one time. Null if there was not the memory. |
| `pool_destroy` | `src/pool.c` | Ends each worker and frees the pool. A null `p` is nothing to free. |
| `pool_run` | `src/pool_run.c` | Runs each job of a batch on the threads of the pool and on the caller's, and returns when all ran. |
| `pool_run_taken` | `src/pool_job.c` | Takes jobs of the run in progress and runs them, until none is left to take. |
| `pool_work` | `src/pool_job.c` | What the thread of a worker runs: it sleeps until woken, runs what it can take, and ends when the pool must quit. |
