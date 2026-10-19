/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>
#include <string.h>

#include "adopt.h"

int adopt_check_write(adopt_t *t, backend_instance_t *one)
{
    int32_t status = -1;

    if (adopt_run(t, one, "write", &status) || status)
        return (adopt_fail("write: %s", t->err.text));
    if (memcmp(t->block + 16, "\x0D\xF0\x0D\x60", 4))
        return (adopt_fail("the host does not see the write"));
    printf("one store: the host reads what the module wrote\n");
    return (0);
}

static void fill(void *arg)
{
    adopt_job_t *j = arg;
    backend_instance_t *i = adopt_make_instance(j->t);
    backend_err_t err = {};
    backend_func_t *f = j->t->api.instance_get_func(i, j->name, &err);
    int32_t status = 0;
    int r = 0;

    while (r < j->rounds) {
        if (!f || j->t->api.call(f, &status, &err) || status)
            j->failed++;
        r++;
    }
    j->t->api.instance_destroy(i);
}

static int count_wrong(const adopt_t *t)
{
    int bad = 0;
    size_t k = 0;

    while (k < 2 * ADOPT_PAGE) {
        bad += t->block[k] != (k < ADOPT_PAGE ? 0xAA : 0xBB);
        k++;
    }
    return (bad);
}

int adopt_check_two_stores(adopt_t *t, backend_instance_t *one)
{
    adopt_job_t low = { t, "fill_low", 200, 0 };
    adopt_job_t high = { t, "fill_high", 200, 0 };
    os_thread_t *a = os_thread_start(fill, &low);
    os_thread_t *b = os_thread_start(fill, &high);
    int32_t status = -1;
    int bad;

    os_thread_join(a);
    os_thread_join(b);
    bad = count_wrong(t);
    if (low.failed || high.failed || bad)
        return (adopt_fail("two stores: %d and %d calls failed, %d bytes "
            "wrong", low.failed, high.failed, bad));
    if (adopt_run(t, one, "sum_high", &status)
        || status != 0xBB * (int32_t)ADOPT_PAGE)
        return (adopt_fail("a store does not see another store's writes "
            "(%d): %s", status, t->err.text));
    printf("two stores, two threads: both halves intact, and each sees the "
        "other's\n");
    return (0);
}

int adopt_check_stray(adopt_t *t, backend_instance_t *one)
{
    int32_t status = -1;

    if (!adopt_run(t, one, "stray", &status))
        return (adopt_fail("a load past the block returned %d instead of "
            "trapping", status));
    printf("stray load: %s\n", t->err.text);
    if (adopt_run(t, one, "write", &status) || status)
        return (adopt_fail("after the trap: %s", t->err.text));
    printf("after the trap: the store still runs\n");
    return (0);
}
