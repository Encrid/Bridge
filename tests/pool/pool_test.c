/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pool_test.h"

constexpr uint32_t MOST_JOBS = 16;

typedef struct {
    atomic_uint runs;
    bool on_caller;
} job_t;

static thread_local bool is_caller;
static job_t jobs[MOST_JOBS];
static void *each[MOST_JOBS];
static uint8_t pinned[MOST_JOBS];

static void run_job(void *arg)
{
    job_t *j = arg;

    atomic_fetch_add(&j->runs, 1);
    j->on_caller = is_caller;
}

static void print_runs(uint32_t count, uint32_t rounds)
{
    uint32_t run = 0;
    uint32_t wanted = 0;
    uint32_t held = 0;
    uint32_t here = 0;

    for (uint32_t i = 0; i < count; i++) {
        run += atomic_load(&jobs[i].runs) == rounds;
        wanted += pinned[i];
        held += pinned[i] && jobs[i].on_caller;
        here += jobs[i].on_caller;
    }
    printf("jobs run in each round: %u of %u\n", (unsigned)run,
        (unsigned)count);
    printf("pinned jobs on the caller: %u of %u\n", (unsigned)held,
        (unsigned)wanted);
    printf("jobs on the caller: %u\n", (unsigned)here);
}

static uint32_t fill_batch(pool_batch_t *batch, const char *flags)
{
    uint32_t count = (uint32_t)strlen(flags);

    if (count > MOST_JOBS)
        count = MOST_JOBS;
    for (uint32_t i = 0; i < count; i++) {
        each[i] = &jobs[i];
        pinned[i] = flags[i] == '1';
    }
    *batch = (pool_batch_t){ run_job, each, pinned, count };
    return (count);
}

static int run_rounds(uint32_t workers, uint32_t rounds, const char *flags)
{
    pool_batch_t batch;
    uint32_t count = fill_batch(&batch, flags);
    pool_t *p = pool_create(workers, count, POOL_TEST_STACK);

    if (!p)
        return (printf("no pool\n"), 0);
    for (uint32_t i = 0; i < rounds; i++)
        pool_run(p, &batch);
    pool_destroy(p);
    print_runs(count, rounds);
    return (0);
}

int main(int argc, char **argv)
{
    const char *mode = argc > 1 ? argv[1] : "";
    uint32_t first = argc > 2 ? (uint32_t)strtoul(argv[2], nullptr, 10) : 0;
    uint32_t second = argc > 3 ? (uint32_t)strtoul(argv[3], nullptr, 10) : 0;

    is_caller = true;
    if (!strcmp(mode, "run") && argc > 3)
        return (run_rounds(first, second, argc > 4 ? argv[4] : ""));
    if (!strcmp(mode, "meet"))
        return (pool_test_meet(first));
    if (strcmp(mode, "none"))
        return (printf("no mode '%s'\n", mode), 2);
    pool_destroy(nullptr);
    printf("no pool to end\n");
    return (0);
}
