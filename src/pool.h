/*
** Copyright (C) 2026 Bridge Owner
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** src/pool.h -- the threads a phase runs on: `threads - 1` workers kept for
** the deck's life, and the caller. Nothing here knows what a job is.
**
** threads.md, *Threads*
*/

#ifndef BRIDGE_SRC_POOL_H
    #define BRIDGE_SRC_POOL_H

    #include <stdint.h>

    #include "os.h"

typedef struct pool pool_t;
typedef void (*pool_fn_t)(void *job);

/*
** What one run is handed: `count` jobs, the function each is given to, and
** for each job whether it is the caller's alone.
*/
typedef struct {
    pool_fn_t fn;
    void **jobs;
    const uint8_t *caller_only;
    uint32_t count;
} pool_batch_t;

typedef struct {
    pool_t *p;
    os_event_t *wake;
    os_thread_t *thread;
} pool_worker_t;

/*
** `run`, `taken` and `left` are the run in progress: all of it is read and
** written under the lock.
*/
struct pool {
    pool_worker_t *workers;
    uint32_t worker_count;
    os_mutex_t *lock;
    os_event_t *done;
    int quit;
    pool_batch_t run;
    uint8_t *taken;
    uint32_t left;
};

/*
** pool_create
**
** Makes a pool of `workers` threads that can run `most_jobs` jobs at once.
** nullptr if there was not the memory for it, with what was made of it by
** then unmade.
**
** failure.md, *Out of memory*
*/
[[nodiscard]] pool_t *pool_create(uint32_t workers, uint32_t most_jobs);

/*
** pool_destroy
**
** Ends every worker and frees the pool. A null `p` is nothing to free.
*/
void pool_destroy(pool_t *p);

/*
** pool_run
**
** Runs every job of a batch and returns when all have. On the pool's threads
** and the caller's; a job marked for the caller alone is run by the caller.
** One job, or no worker to share with: no thread is woken for it.
*/
void pool_run(pool_t *p, const pool_batch_t *batch);

/*
** pool_run_taken
**
** Takes jobs of the run in progress and runs them, until none is left to
** take. `caller` says whether the jobs marked for the caller may be taken.
*/
void pool_run_taken(pool_t *p, int caller);

/*
** pool_work
**
** What a worker's thread runs: it sleeps until woken, runs what it can take,
** and ends when the pool is told to quit.
*/
void pool_work(void *arg);

#endif
