/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "cases.h"

void test_reload_group_stop(host_t *h)
{
    bridge_artifact_t a[2];
    bridge_artifact_t sour;

    if (host_open(h))
        return;
    test_state_guarded(&sour, "view:sour@0.1.0", "sour");
    sour.cfg.read = TEST_GAME;
    sour.cfg.read_count = 2;
    sour.cfg.max_system_failures = 1;
    sour.cfg.on_error = BRIDGE_FAIL;
    host_load(h, &sour);
    group_state(&a[1], "game:one@0.1.0", "share1");
    host_load(h, &a[1]);
    group_state(&a[0], "game:marker@0.1.0", "marker");
    host_load(h, &a[0]);
    group_state(&a[0], "game:marker@0.2.0", "marker");
    group_state(&a[1], "game:one@0.2.0", "share1");
    host_reload_many(h, a, 2);
    test_say_modules(h);
    host_close(h);
}

void test_reload_group_hold(host_t *h)
{
    bridge_artifact_t a[2];

    if (host_open(h))
        return;
    group_state(&a[0], "game:one@0.1.0", "share1");
    group_state(&a[1], "game:two@0.1.0", "sharehold0");
    host_load(h, &a[0]);
    host_load(h, &a[1]);
    host_run_frames(h, 1);
    group_state(&a[0], "game:one@0.2.0", "share2");
    group_state(&a[1], "game:two@0.2.0", "sharehold1");
    host_reload_many(h, a, 2);
    host_run_frames(h, 1);
    test_say_modules(h);
    host_close(h);
}
