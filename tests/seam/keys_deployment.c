/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <string.h>

#include "cases.h"

constexpr int64_t BEYOND = 1ll << 24;
constexpr int64_t PAST = 1ll << 32;

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
    { "sparse_sets=past", offsetof(bridge_config_t, budget.sparse_sets),
        8, PAST },
    { "persistent=past", offsetof(bridge_config_t, budget.persistent), 8,
        PAST },
    { "columns=past", offsetof(bridge_config_t, budget.columns), 8, PAST },
    { "direct_indexed=0", offsetof(bridge_config_t, budget.direct_indexed),
        8, 0 },
    { "direct_indexed=no limit",
        offsetof(bridge_config_t, budget.direct_indexed), 8, BRIDGE_NO_LIMIT },
    { "direct_indexed=beyond",
        offsetof(bridge_config_t, budget.direct_indexed), 8, BEYOND },
    { "profile=0", offsetof(bridge_config_t, profile), 1, 0 },
    { "profile=3", offsetof(bridge_config_t, profile), 1, 3 },
    { "validate=0", offsetof(bridge_config_t, validate), 1, 0 },
    { "validate=3", offsetof(bridge_config_t, validate), 1, 3 },
    { "stack=0", offsetof(bridge_config_t, stack), 8, 0 },
    { "table_growth=0", offsetof(bridge_config_t, table_growth), 4, 0 },
    { "table_growth=99", offsetof(bridge_config_t, table_growth), 4, 99 },
    { "table_growth=100", offsetof(bridge_config_t, table_growth), 4, 100 },
    { "table_hash_fill=0", offsetof(bridge_config_t, table_hash_fill), 4, 0 },
    { "table_hash_fill=99", offsetof(bridge_config_t, table_hash_fill), 4,
        99 },
    { "table_hash_fill=100", offsetof(bridge_config_t, table_hash_fill), 4,
        100 },
    { "observer_passes=0", offsetof(bridge_config_t, observer_passes), 4, 0 },
    { "table_retry=0", offsetof(bridge_config_t, table_retry), 4, 0 },
    { "table_retry=99", offsetof(bridge_config_t, table_retry), 4, 99 },
    { "table_retry=100", offsetof(bridge_config_t, table_retry), 4, 100 }
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

void test_edit_backend(host_t *h)
{
    if (!strcmp(h->arg, "name"))
        h->backends[1].name = nullptr;
    if (!strcmp(h->arg, "library"))
        h->backends[1].library = nullptr;
    if (!strcmp(h->arg, "twice"))
        h->backends[1].name = h->backends[0].name;
    if (!strcmp(h->arg, "hollow"))
        h->backends[1].library = HOLLOW_PATH;
    if (!strcmp(h->arg, "refusing"))
        h->backends[1] = (bridge_backend_t){ "plain", PLAIN_REFUSING_PATH };
    if (!strcmp(h->arg, "module"))
        h->backends[1].library = OK_PATH;
    if (!strcmp(h->arg, "untold"))
        h->backends[1] = (bridge_backend_t){ "plain", TESTS_PATH };
    host_open_only(h);
}

void test_edit_phases(host_t *h)
{
    h->phases[1] = strcmp(h->arg, "twice") ? nullptr : h->phases[0];
    if (!strcmp(h->arg, "empty"))
        h->phases[1] = "";
    host_open_only(h);
}
