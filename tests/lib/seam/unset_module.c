/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "cases.h"

void test_state_no_host_abi(host_t *h)
{
    h->cfg.abi = 0;
    if (host_open(h))
        return;
    test_load_small(h, h->arg);
    host_close(h);
}

void test_leave_unset(host_t *h)
{
    char identity[64];
    bridge_artifact_t a;

    if (host_open(h))
        return;
    test_state_small(&a, identity, "ok");
    a.cfg.memory_limit = 0;
    host_load(h, &a);
    host_close(h);
}

void test_leave_level_unset(host_t *h)
{
    char identity[64];
    bridge_artifact_t a;

    if (host_open(h))
        return;
    test_state_small(&a, identity, "ok");
    a.cfg.log_level = 0;
    host_load(h, &a);
    host_close(h);
}

void test_leave_flag_unset(host_t *h)
{
    char identity[64];
    bridge_artifact_t a;

    if (host_open(h))
        return;
    test_state_small(&a, identity, "ok");
    a.cfg.pinned = BRIDGE_UNSET;
    host_load(h, &a);
    test_state_small(&a, identity, "ok");
    a.cfg.hot_reload = (enum bridge_flag)3;
    host_load(h, &a);
    host_close(h);
}

void test_state_negative(host_t *h)
{
    char identity[64];
    bridge_artifact_t a;

    if (host_open(h))
        return;
    test_state_small(&a, identity, "ok");
    a.cfg.memory_limit = -7;
    host_load(h, &a);
    host_close(h);
}
