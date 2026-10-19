/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "cases.h"

void test_state_no_limit(host_t *h)
{
    char identity[64];
    bridge_artifact_t a;

    if (host_open(h))
        return;
    test_state_small(&a, identity, "ok");
    a.cfg.work_limit = BRIDGE_NO_LIMIT;
    a.cfg.memory_limit = BRIDGE_NO_LIMIT;
    a.cfg.init_timeout = BRIDGE_NO_LIMIT;
    a.cfg.reload_timeout = BRIDGE_NO_LIMIT;
    a.cfg.shutdown_timeout = BRIDGE_NO_LIMIT;
    host_load(h, &a);
    host_close(h);
}

void test_state_other_none(host_t *h)
{
    char identity[64];
    bridge_artifact_t a;

    if (host_open(h))
        return;
    test_state_small(&a, identity, "ok");
    a.cfg.spawns = BRIDGE_NO_LIMIT;
    host_load(h, &a);
    test_state_small(&a, identity, "ok");
    a.cfg.work_limit = BRIDGE_NONE;
    host_load(h, &a);
    host_close(h);
}
