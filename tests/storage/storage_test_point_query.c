/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>
#include <string.h>

#include "bridge_abi.h"
#include "storage_test.h"

static uint32_t find_live_query(const module_t *m)
{
    const bridge_query_t *q = (const bridge_query_t *)(m->priv + m->spans);
    uint32_t i = 0;

    while (i < m->system_count && !q[i].spans)
        i++;
    return (i);
}

static void point_query(module_t *m, uint32_t i, uint32_t at)
{
    bridge_query_t *q = (bridge_query_t *)(m->priv + m->spans);
    uint32_t zero = 0;

    memcpy(m->priv + at, &zero, 4);
    q[i].at = at;
}

static uint32_t read_at(const module_t *m, uint32_t at)
{
    uint32_t got;

    memcpy(&got, m->priv + at, 4);
    return (got);
}

enum storage_test_end storage_test_point_query_away(storage_test_t *t)
{
    bridge_artifact_t a = storage_test_state(STORE_PATH, 6, 1 << 20, 1 << 20);
    module_t *m;
    uint32_t at;
    uint32_t i;

    if (storage_test_load(t, &a))
        return (STORAGE_TEST_FAILED);
    storage_test_run(t, "spawn");
    storage_test_run(t, "attach_a");
    m = t->b->modules;
    i = find_live_query(m);
    at = m->scratch + m->scratch_len / 2;
    if (i < m->system_count)
        point_query(m, i, at);
    storage_test_run(t, "spawn");
    storage_test_run(t, "attach_a");
    printf("outside spans [%u, %u): wrote %u at %u\n", m->spans,
        m->spans + m->spans_len, read_at(m, at), at);
    return (STORAGE_TEST_DONE);
}
