/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "cases.h"

void test_stop_by_barred(host_t *h)
{
    bridge_artifact_t a;

    if (host_open(h))
        return;
    test_load_built(h, "game:stamp5@0.1.0", STAMP5_PATH, 8);
    host_state_built(&a, "view:reaper@0.1.0", REAPER_PATH);
    a.cfg.read = TEST_GAME;
    a.cfg.read_count = 1;
    a.cfg.max_system_failures = 1;
    host_load(h, &a);
    host_run_frames(h, 1);
    test_say_modules(h);
    host_close(h);
}

void test_stop_two_observers(host_t *h)
{
    bridge_artifact_t a;

    if (host_open(h))
        return;
    test_load_built(h, "game:stamp5@0.1.0", STAMP5_PATH, 8);
    test_state_guarded(&a, "view:sour@0.1.0", "sour");
    a.cfg.read = TEST_GAME;
    a.cfg.read_count = 2;
    a.cfg.max_system_failures = 1;
    host_load(h, &a);
    host_run_frames(h, 1);
    test_say_modules(h);
    host_close(h);
}

void test_mend_after_fault(host_t *h)
{
    bridge_artifact_t a;

    if (host_open(h))
        return;
    test_load_built(h, "game:stamp5@0.1.0", STAMP5_PATH, 8);
    host_state_plain(&a, "view:reaper@0.1.0", nullptr);
    host_name_module(&a, "reaper-mend");
    a.cfg.spawns = 4;
    a.cfg.read = TEST_GAME;
    a.cfg.read_count = 1;
    a.cfg.max_system_failures = 2;
    host_load(h, &a);
    host_run_frames(h, 3);
    test_say_modules(h);
    host_close(h);
}
