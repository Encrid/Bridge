/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>
#include <stdlib.h>

#include "storage_test.h"

constexpr uint32_t STORAGE_TEST_FRAMES = 6'144;

static os_event_t *started;

static uint32_t go_deep(uint32_t depth, volatile uint8_t *above)
{
    volatile uint8_t frame[4'096];

    frame[0] = (uint8_t)(above[0] + 1);
    if (!depth)
        return (frame[0]);
    return (go_deep(depth - 1, frame));
}

static void run_job(void *job)
{
    volatile uint8_t top[1] = {};

    if (!job) {
        os_event_wait(started, OS_FOREVER);
        return;
    }
    os_event_set(started);
    printf("a call of %u frames of 4096 bytes gave %u on a worker\n",
        STORAGE_TEST_FRAMES, go_deep(STORAGE_TEST_FRAMES, top));
}

enum storage_test_end storage_test_go_deep(storage_test_t *t)
{
    int worker = 1;
    void *jobs[2] = { nullptr, &worker };
    const uint8_t caller_only[2] = { 1, 0 };
    const pool_batch_t batch = { run_job, jobs, caller_only, 2 };

    (void)bridge_destroy(t->w);
    t->cfg.stack = strtoull(t->more, nullptr, 10);
    t->w = bridge_create(&t->cfg, storage_test_hear, t);
    started = os_event_create();
    if (!t->w || !started)
        return (STORAGE_TEST_FAILED);
    pool_run(t->w->pool, &batch);
    os_event_free(started);
    return (STORAGE_TEST_DONE);
}
