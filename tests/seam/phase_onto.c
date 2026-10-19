/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "cases.h"

void test_map_onto_a_named_phase(host_t *h)
{
    bridge_phase_name_t tick = { "tick", "update" };
    bridge_artifact_t a;

    if (host_open(h))
        return;
    test_state_guarded(&a, "game:twostep@0.1.0", "twostep");
    a.cfg.phases = &tick;
    a.cfg.phase_count = 1;
    host_load(h, &a);
    host_close(h);
}

void test_map_beside_a_named_phase(host_t *h)
{
    bridge_phase_name_t tick = { "tick", "present" };
    bridge_artifact_t a;

    if (host_open(h))
        return;
    test_state_guarded(&a, "game:places@0.1.0", "places");
    a.cfg.phases = &tick;
    a.cfg.phase_count = 1;
    host_load(h, &a);
    host_run_frames_unsaid(h->b, 1);
    host_close(h);
}
