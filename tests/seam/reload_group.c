/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <string.h>

#include "cases.h"

void group_state(bridge_artifact_t *a, const char *identity, const char *file)
{
    test_state_guarded(a, identity, file);
    a->cfg.reloadable = BRIDGE_TRUE;
    a->cfg.hot_reload = BRIDGE_TRUE;
    a->cfg.spawns = 8;
}

void test_reload_group(host_t *h)
{
    const char *to = strcmp(h->arg, "shared") ? "share1" : "share2";
    bridge_artifact_t a[2];

    if (host_open(h))
        return;
    group_state(&a[0], "game:one@0.1.0", "share1");
    group_state(&a[1], "game:two@0.1.0", "share1");
    host_load(h, &a[0]);
    host_load(h, &a[1]);
    host_run_frames(h, 1);
    group_state(&a[0], "game:one@0.2.0", to);
    group_state(&a[1], "game:two@0.2.0", to);
    host_reload_many(h, a, 2);
    host_run_frames(h, 1);
    test_say_modules(h);
    host_close(h);
}

void test_reload_group_outside(host_t *h)
{
    bridge_artifact_t a[2];
    bridge_artifact_t c;

    if (host_open(h))
        return;
    group_state(&a[0], "game:one@0.1.0", "share1");
    group_state(&a[1], "game:two@0.1.0", "share1");
    group_state(&c, "game:three@0.1.0", "share1");
    host_load(h, &a[0]);
    host_load(h, &a[1]);
    host_load(h, &c);
    group_state(&a[0], "game:one@0.2.0", "share2");
    group_state(&a[1], "game:two@0.2.0", "share2");
    host_reload_many(h, a, 2);
    test_say_modules(h);
    host_close(h);
}

void test_reload_group_start(host_t *h)
{
    bridge_artifact_t a[2];

    if (host_open(h))
        return;
    group_state(&a[0], "game:one@0.1.0", "share1");
    group_state(&a[1], "game:two@0.1.0", "share1");
    host_load(h, &a[0]);
    host_load(h, &a[1]);
    host_run_frames(h, 1);
    group_state(&a[0], "game:one@0.2.0", "share2");
    group_state(&a[1], "game:two@0.2.0", "sharebad");
    host_reload_many(h, a, 2);
    test_say_modules(h);
    host_close(h);
}

void test_reload_group_cycle(host_t *h)
{
    bridge_artifact_t a[2];

    if (host_open(h))
        return;
    group_state(&a[0], "game:one@0.1.0", "share1");
    group_state(&a[1], "game:two@0.1.0", "share1");
    host_load(h, &a[0]);
    host_load(h, &a[1]);
    group_state(&a[0], "game:one@0.2.0", "sharew");
    group_state(&a[1], "game:two@0.2.0", "sharew");
    host_reload_many(h, a, 2);
    test_say_modules(h);
    host_close(h);
}
