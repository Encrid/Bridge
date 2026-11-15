/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdatomic.h>
#include <stdio.h>

#include "pool_test.h"

typedef struct {
    os_event_t *here;
    os_event_t *other;
    atomic_bool came;
    const atomic_bool *other_came;
    bool met;
} meet_t;

static void meet_other(void *arg)
{
    meet_t *m = arg;

    atomic_store(&m->came, true);
    os_event_set(m->here);
    os_event_wait(m->other, 2 * OS_SECOND);
    m->met = atomic_load(m->other_came);
}

static int run_meeting(uint32_t workers, meet_t *two)
{
    void *jobs[2] = { &two[0], &two[1] };
    const uint8_t pinned[2] = {};
    pool_batch_t batch = { meet_other, jobs, pinned, 2 };
    pool_t *p = pool_create(workers, 2, POOL_TEST_STACK);

    if (!p)
        return (printf("no pool\n"), 1);
    pool_run(p, &batch);
    pool_destroy(p);
    return (0);
}

int pool_test_meet(uint32_t workers)
{
    meet_t two[2] = {};
    int failed = 1;

    two[0].here = os_event_create();
    two[1].here = os_event_create();
    two[0].other = two[1].here;
    two[1].other = two[0].here;
    two[0].other_came = &two[1].came;
    two[1].other_came = &two[0].came;
    if (two[0].here && two[1].here)
        failed = run_meeting(workers, two);
    printf("the 2 jobs %s\n",
        two[0].met && two[1].met ? "met" : "did not meet");
    os_event_free(two[0].here);
    os_event_free(two[1].here);
    return (failed);
}
