/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "cases.h"

static void load_ticking(host_t *h, const bridge_phase_name_t *tick)
{
    bridge_artifact_t a;

    if (host_open(h))
        return;
    host_state_built(&a, "game:stamp@0.1.0", STAMPTICK_PATH);
    a.cfg.spawns = 8;
    a.cfg.phases = tick;
    a.cfg.phase_count = tick ? 1 : 0;
    host_load(h, &a);
    test_load_watcher(h);
    host_run_frames_unsaid(h->b, 2);
    host_close(h);
}

void test_leave_phase_unmapped(host_t *h)
{
    load_ticking(h, nullptr);
}

void test_map_phase(host_t *h)
{
    bridge_phase_name_t tick = { "tick", "update" };

    load_ticking(h, &tick);
}

void test_map_phase_nowhere(host_t *h)
{
    bridge_phase_name_t tick = { "tick", "nowhere" };

    load_ticking(h, &tick);
}
