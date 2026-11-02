/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdlib.h>

#include "cases.h"

static const bridge_order_t TEST_PAIRS[2] = {
    { "game:mover:move", "view:halver:halve" },
    { "view:halver:halve", "game:mover:move" }
};

static void load_three(host_t *h)
{
    bridge_artifact_t a;

    if (host_open(h))
        return;
    test_load_guarded(h, "game:mover@0.1.0", "mover");
    test_state_guarded(&a, "view:halver@0.1.0", "halver");
    a.cfg.write = TEST_GAME;
    a.cfg.write_count = 1;
    host_load(h, &a);
    test_load_game_reader(h);
    host_run_frames_unsaid(h->w, 1);
    host_close(h);
}

void test_order(host_t *h)
{
    char *count = nullptr;

    h->cfg.order = &TEST_PAIRS[strtol(h->arg, &count, 10)];
    h->cfg.order_count = (uint32_t)strtol(count, nullptr, 10);
    load_three(h);
}
