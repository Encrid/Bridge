/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdlib.h>
#include <string.h>

#include "cases.h"

static const char *const TEST_LONG_DECLARERS[2] = {
    "game:mover_with_a_long_name_that_fills_the_list_of_declarers@0.1.0",
    "view:reader_with_a_long_name_that_fills_the_list_of_declarers@0.1.0"
};

static const char *const TEST_SHORT_DECLARERS[2] = {
    "game:mover@0.1.0",
    "view:reader@0.1.0"
};

void test_watch_spans(host_t *h)
{
    bridge_artifact_t a;

    if (host_open(h))
        return;
    test_state_guarded(&a, "game:watcher@0.1.0", "watcher");
    a.cfg.spans = (uint32_t)strtoul(h->arg, nullptr, 10);
    host_load(h, &a);
    host_close(h);
}

void test_name_many_declarers(host_t *h)
{
    const char *const *names = TEST_LONG_DECLARERS;
    bridge_artifact_t a;

    if (!strcmp(h->arg, "short"))
        names = TEST_SHORT_DECLARERS;
    if (host_open(h))
        return;
    test_load_guarded(h, names[0], "mover");
    test_state_guarded(&a, names[1], "reader");
    a.cfg.read = TEST_GAME;
    a.cfg.read_count = 2;
    host_load(h, &a);
    test_load_guarded(h, "game:duo@0.1.0", "duo");
    test_load_guarded(h, "game:fp@0.1.0", "fp");
    host_close(h);
}

static void run_duo(host_t *h, const char *file, int64_t spawns)
{
    bridge_artifact_t a;

    test_state_guarded(&a, "game:duo@0.1.0", file);
    a.cfg.spawns = spawns;
    host_load(h, &a);
    host_run_frames(h, 1);
}

void test_two_turns_of_one_module(host_t *h)
{
    if (host_open(h))
        return;
    run_duo(h, "duo", strtoll(h->arg, nullptr, 10));
    test_say_modules(h);
    host_close(h);
}

void test_walk_optional(host_t *h)
{
    if (host_open(h))
        return;
    run_duo(h, h->arg, 8);
    host_close(h);
}

void test_pick_sparse(host_t *h)
{
    h->cfg.budget.sparse_sets = BRIDGE_NONE;
    if (strcmp(h->arg, "none"))
        h->cfg.budget.sparse_sets = strtoll(h->arg, nullptr, 10);
    if (host_open(h))
        return;
    test_load_guarded(h, "game:pick@0.1.0", "pick");
    host_run_frames(h, 1);
    test_say_modules(h);
    host_close(h);
}

void test_hear_singleton_twice(host_t *h)
{
    bridge_artifact_t a;

    if (host_open(h))
        return;
    test_load_guarded(h, "game:hearer@0.1.0", "hearer");
    test_state_guarded(&a, "game:swapper@0.1.0", "swapper");
    a.cfg.reloadable = BRIDGE_TRUE;
    a.cfg.hot_reload = BRIDGE_TRUE;
    host_load(h, &a);
    test_state_guarded(&a, "game:swapper@0.2.0", h->arg);
    a.cfg.reloadable = BRIDGE_TRUE;
    a.cfg.hot_reload = BRIDGE_TRUE;
    host_reload(h, &a);
    host_run_frames(h, 1);
    host_close(h);
}
