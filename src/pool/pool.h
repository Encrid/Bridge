/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#ifndef BRIDGE_SRC_POOL_H
    #define BRIDGE_SRC_POOL_H

    #include <stddef.h>
    #include <stdint.h>

    #include "os.h"

typedef struct pool pool_t;
typedef void (*pool_fn_t)(void *job);

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

[[nodiscard]] pool_t *pool_create(uint32_t workers, uint32_t most_jobs,
    size_t stack);

void pool_destroy(pool_t *p);

void pool_run(pool_t *p, const pool_batch_t *batch);

void pool_run_alone(const pool_batch_t *batch);

void pool_run_taken(pool_t *p, int caller);

void pool_work(void *arg);

#endif
