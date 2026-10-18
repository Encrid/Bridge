# src/lib/pool/pool.h

This header gives the pool that runs a phase: `threads - 1` workers kept for the life of the world, and the caller.
Nothing in the pool knows what a job is.

## The execution model

A phase runs its systems as `docs/extern/bridge.h.md`, *bridge_run_phase*, tells.
In each level, the schedule makes one job of the systems of each module, and one thread runs each job: `docs/intern/lib/schedule/schedule.h.md`, *Levels and jobs*.
The schedule finds the order a system at a time, and what runs at the same time a module at a time: `src/lib/schedule/schedule.h`.

The systems of one module cannot overlap, because they share what only one of them may use.
On the wasm backend, a module is one instance in one store, and one thread at a time enters a store.
On each backend, a module has one private memory.
Before each call, the library writes there which system runs, and the module moves one scratch pointer and one log ring head there.
On the native backend, a module also has its own globals, and nothing guards them.

So one heavy system cannot use more than one core.
A deployment gets parallel work from more modules whose systems do not conflict, and not from more systems.
One loop over one component cannot be split between two modules either: both would write it, conflict, and be put in order.

`threads` is the size of the pool, with the caller: `docs/extern/bridge.h.md`, *bridge_config_t*.

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
| `pool_create` | `src/lib/pool/pool.c` | Makes a pool: `docs/intern/lib/pool/pool.c.md`, *Functions*. |
| `pool_destroy` | `src/lib/pool/pool.c` | Frees a pool: `docs/intern/lib/pool/pool.c.md`, *Functions*. |
| `pool_run` | `src/lib/pool/pool_run.c` | Runs a batch of jobs: `docs/intern/lib/pool/pool_run.c.md`, *Functions*. |
| `pool_run_taken` | `src/lib/pool/pool_job.c` | Runs the jobs that a thread can take: `docs/intern/lib/pool/pool_job.c.md`, *Functions*. |
| `pool_work` | `src/lib/pool/pool_job.c` | Runs the thread of a worker: `docs/intern/lib/pool/pool_job.c.md`, *Functions*. |
