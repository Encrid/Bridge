/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "cases.h"

static void run_ender(host_t *h, const char *file, int frames)
{
    bridge_artifact_t a;

    if (host_open(h))
        return;
    test_state_guarded(&a, "test:ender@0.1.0", file);
    a.cfg.max_system_failures = 1;
    host_load(h, &a);
    host_run_frames(h, frames);
    test_say_modules(h);
    host_close(h);
}

void test_end_by_system(host_t *h)
{
    run_ender(h, "ender", 3);
}

void test_end_by_init(host_t *h)
{
    run_ender(h, "ender-init", 1);
}

void test_end_by_observer(host_t *h)
{
    bridge_artifact_t a;

    if (host_open(h))
        return;
    test_load_built(h, "game:stamp5@0.1.0", STAMP5_PATH, 8);
    test_state_guarded(&a, "view:ender@0.1.0", "ender-heard");
    a.cfg.read = TEST_GAME;
    a.cfg.read_count = 2;
    host_load(h, &a);
    host_run_frames(h, 1);
    test_say_modules(h);
    host_close(h);
}

void test_end_at_a_reload(host_t *h)
{
    bridge_artifact_t a;

    if (host_open(h))
        return;
    test_state_guarded(&a, "view:ender@0.1.0", "ender-heard");
    a.cfg.read = TEST_GAME;
    a.cfg.read_count = 2;
    host_load(h, &a);
    test_state_guarded(&a, "game:marker@0.1.0", "marker");
    a.cfg.reloadable = BRIDGE_TRUE;
    a.cfg.hot_reload = BRIDGE_TRUE;
    a.cfg.spawns = 4;
    host_load(h, &a);
    a.cfg.identity = "game:marker@0.2.0";
    host_reload(h, &a);
    host_run_frames(h, 1);
    host_close(h);
}
