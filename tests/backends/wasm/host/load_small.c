/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>

#include "cases.h"

void test_print([[maybe_unused]] void *ud, const bridge_record_t *r)
{
    printf("record[%d]: %.*s\n", r->level, (int)r->len, r->text);
}

void test_state_small(bridge_artifact_t *a, char *identity, const char *name)
{
    snprintf(identity, 64, "test:%s@0.1.0", name);
    host_state_wasm(a, identity, name);
    a->cfg.work_limit = 1'000'000;
    a->cfg.memory_limit = 1 << 20;
    a->cfg.init_timeout = 200 * HOST_MS;
    a->cfg.reload_timeout = 200 * HOST_MS;
    a->cfg.shutdown_timeout = 200 * HOST_MS;
    a->cfg.scratch = 4096;
    a->cfg.log_ring = 240;
    a->cfg.spans = 4096;
    a->cfg.changes = 4096;
}

void test_load_small(host_t *h, const char *name)
{
    char identity[64];
    bridge_artifact_t a;

    test_state_small(&a, identity, name);
    host_load(h, &a);
}

void test_load_named(host_t *h)
{
    if (host_open(h))
        return;
    test_load_small(h, h->arg);
    host_close(h);
}

void test_stop(host_t *h)
{
    char identity[64];
    bridge_artifact_t a;

    if (host_open(h))
        return;
    test_load_small(h, "oob");
    test_load_small(h, "ok");
    test_state_small(&a, identity, "oob");
    a.cfg.on_error = BRIDGE_FAIL;
    host_load(h, &a);
    test_load_small(h, "ok");
    host_close(h);
}
