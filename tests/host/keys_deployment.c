/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge-ASM700.
**
** All rights reserved.
**
** tests/host/keys_deployment.c -- a key of the deployment stated as what
** is refused: each a row, the key and the value it is given.
*/

#include <string.h>

#include "cases.h"

constexpr int64_t BEYOND = 1ll << 24;

static const test_edit_t TEST_DEPLOYMENT_EDITS[] = {
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

/*
** test_put_key
**
** Writes a value into a key of `size` bytes. Every system this is built
** for keeps the low byte first, so the low bytes of the value are the key.
*/
void test_put_key(void *key, size_t size, int64_t value)
{
    memcpy(key, &value, size);
}

/*
** test_find_edit
**
** Answers the row of a name, among rows the caller knows hold it.
*/
const test_edit_t *test_find_edit(const test_edit_t *rows, const char *name)
{
    while (strcmp(rows->name, name))
        rows++;
    return (rows);
}

/*
** test_edit_deployment
**
** A deployment with one key stated as the case's row says: it is refused.
*/
void test_edit_deployment(host_t *h)
{
    const test_edit_t *e = test_find_edit(TEST_DEPLOYMENT_EDITS, h->arg);

    test_put_key((char *)&h->cfg + e->at, e->size, e->value);
    host_open_only(h);
}

/*
** test_edit_backend
**
** A backend with no name, with no library, or named as the other is; and
** one whose library lacks an entry, or adopts no block.
*/
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
        h->backends[1].library = NATIVE_REFUSING_PATH;
    host_open_only(h);
}

/*
** test_edit_phases
**
** A phase with no name, or named as the other is.
*/
void test_edit_phases(host_t *h)
{
    h->phases[1] = strcmp(h->arg, "twice") ? nullptr : h->phases[0];
    host_open_only(h);
}
