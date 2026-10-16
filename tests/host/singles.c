/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "cases.h"

void test_watch_no_room(host_t *h)
{
    bridge_artifact_t a;

    if (host_open(h))
        return;
    test_state_guarded(&a, "game:watcher@0.1.0", "watcher");
    a.cfg.spans = 8;
    host_load(h, &a);
    host_close(h);
}

void test_name_many_declarers(host_t *h)
{
    bridge_artifact_t a;

    if (host_open(h))
        return;
    test_load_guarded(h,
        "game:mover_with_a_long_name_that_fills_the_list_of_declarers@0.1.0",
        "mover");
    test_state_guarded(&a,
        "view:reader_with_a_long_name_that_fills_the_list_of_declarers@0.1.0",
        "reader");
    a.cfg.read = TEST_GAME;
    a.cfg.read_count = 2;
    host_load(h, &a);
    test_load_guarded(h, "game:duo@0.1.0", "duo");
    test_load_guarded(h, "game:fp@0.1.0", "fp");
    host_close(h);
}

void test_two_turns_of_one_module(host_t *h)
{
    bridge_artifact_t a;

    if (host_open(h))
        return;
    test_state_guarded(&a, "game:duo@0.1.0", "duo");
    a.cfg.spawns = 8;
    host_load(h, &a);
    host_run_frames(h, 1);
    test_say_modules(h);
    host_close(h);
}
