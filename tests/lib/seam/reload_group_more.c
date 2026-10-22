/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "cases.h"

static void group_plain(bridge_artifact_t *a, const char *identity)
{
    host_state_plain(a, identity, nullptr);
    host_name_module(a, "share1");
    a->cfg.reloadable = BRIDGE_TRUE;
    a->cfg.hot_reload = BRIDGE_TRUE;
    a->cfg.spawns = 8;
}

void test_reload_group_plain(host_t *h)
{
    bridge_artifact_t a[2];

    if (host_open(h))
        return;
    group_state(&a[0], "game:one@0.1.0", "share1");
    group_plain(&a[1], "game:two@0.1.0");
    host_load(h, &a[0]);
    host_load(h, &a[1]);
    group_state(&a[0], "game:one@0.2.0", "share1");
    group_plain(&a[1], "game:two@0.2.0");
    host_reload_many(h, a, 2);
    test_say_modules(h);
    host_close(h);
}

void test_reload_group_empty(host_t *h)
{
    bridge_artifact_t a;

    if (host_open(h))
        return;
    group_state(&a, "game:one@0.1.0", "share1");
    host_load(h, &a);
    host_reload_many(h, &a, 0);
    test_say_modules(h);
    host_close(h);
}

void test_reload_group_dup(host_t *h)
{
    bridge_artifact_t a[2];

    if (host_open(h))
        return;
    group_state(&a[0], "game:one@0.1.0", "share1");
    host_load(h, &a[0]);
    group_state(&a[0], "game:one@0.2.0", "share2");
    group_state(&a[1], "game:one@0.2.0", "share2");
    host_reload_many(h, a, 2);
    test_say_modules(h);
    host_close(h);
}

void test_reload_group_ghost(host_t *h)
{
    bridge_artifact_t a[2];

    if (host_open(h))
        return;
    group_state(&a[0], "game:one@0.1.0", "share1");
    host_load(h, &a[0]);
    group_state(&a[0], "game:one@0.2.0", "share2");
    group_state(&a[1], "game:ghost@0.2.0", "share2");
    host_reload_many(h, a, 2);
    test_say_modules(h);
    host_close(h);
}
