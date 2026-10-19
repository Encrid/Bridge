/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <string.h>

#include "cases.h"

constexpr int64_t BEYOND = 1ll << 24;

static const test_edit_t TEST_DEPLOYMENT_EDITS[] = {
    { "backends=0", offsetof(bridge_config_t, backend_count), 4, 0 },
    { "entities=0", offsetof(bridge_config_t, budget.entities), 8, 0 },
    { "modules=0", offsetof(bridge_config_t, budget.modules), 4, 0 },
    { "persistent=0", offsetof(bridge_config_t, budget.persistent), 8, 0 },
    { "sparse_sets=0", offsetof(bridge_config_t, budget.sparse_sets), 8, 0 },
    { "sparse_sets=-5", offsetof(bridge_config_t, budget.sparse_sets), 8,
        -5 },
    { "sparse_sets=beyond", offsetof(bridge_config_t, budget.sparse_sets),
        8, BEYOND },
    { "direct_indexed=0", offsetof(bridge_config_t, budget.direct_indexed),
        8, 0 },
    { "direct_indexed=no limit",
        offsetof(bridge_config_t, budget.direct_indexed), 8, BRIDGE_NO_LIMIT },
    { "direct_indexed=beyond",
        offsetof(bridge_config_t, budget.direct_indexed), 8, BEYOND }
};

void test_put_key(void *key, size_t size, int64_t value)
{
    memcpy(key, &value, size);
}

const test_edit_t *test_find_edit(const test_edit_t *rows, const char *name)
{
    while (strcmp(rows->name, name))
        rows++;
    return (rows);
}

void test_edit_deployment(host_t *h)
{
    const test_edit_t *e = test_find_edit(TEST_DEPLOYMENT_EDITS, h->arg);

    test_put_key((char *)&h->cfg + e->at, e->size, e->value);
    host_open_only(h);
}

void test_edit_phases(host_t *h)
{
    h->phases[1] = strcmp(h->arg, "twice") ? nullptr : h->phases[0];
    host_open_only(h);
}
