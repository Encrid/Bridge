/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "cases.h"

void test_map_named_phase(host_t *h)
{
    bridge_phase_name_t tick = { "tick", h->arg };
    bridge_artifact_t a;

    if (host_open(h))
        return;
    test_state_guarded(&a, "game:places@0.1.0", "places");
    a.cfg.phases = &tick;
    a.cfg.phase_count = 1;
    host_load(h, &a);
    host_run_frames_unsaid(h->w, 1);
    host_close(h);
}
