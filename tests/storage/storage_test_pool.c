/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>
#include <stdlib.h>

#include "storage_test.h"

constexpr uint32_t POOL_JOBS = 64;

static uint32_t runs[POOL_JOBS];
static uint32_t strays;
static uint64_t caller;

static void count_run(void *job)
{
    uint32_t *run = job;

    (*run)++;
    if (!((run - runs) % 4) && os_get_thread_id() != caller)
        strays++;
}

enum storage_test_end storage_test_run_pool(storage_test_t *t)
{
    void *jobs[POOL_JOBS];
    uint8_t caller_only[POOL_JOBS];
    const pool_batch_t batch = { count_run, jobs, caller_only, POOL_JOBS };
    uint32_t rounds = (uint32_t)strtoul(t->more, nullptr, 10);
    uint32_t right = 0;

    caller = os_get_thread_id();
    for (uint32_t i = 0; i < POOL_JOBS; i++) {
        jobs[i] = &runs[i];
        caller_only[i] = !(i % 4);
    }
    for (uint32_t r = 0; r < rounds; r++)
        pool_run(t->w->pool, &batch);
    for (uint32_t i = 0; i < POOL_JOBS; i++)
        right += runs[i] == rounds;
    printf("%u of %u jobs ran %u times each, and %u runs of a job of the "
        "caller were on a worker\n", right, POOL_JOBS, rounds, strays);
    return (STORAGE_TEST_DONE);
}
