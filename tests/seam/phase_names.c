/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>

#include "cases.h"

static void load_mapped(host_t *h, const bridge_phase_name_t *names,
    uint32_t count)
{
    bridge_artifact_t a;

    if (host_open(h))
        return;
    host_state_built(&a, "game:stamp@0.1.0", STAMPTICK_PATH);
    a.cfg.spawns = 8;
    a.cfg.phases = names;
    a.cfg.phase_count = count;
    host_load(h, &a);
    test_load_watcher(h);
    host_run_frames_unsaid(h->w, 2);
    host_close(h);
}

void test_map_phase(host_t *h)
{
    char to[2][16] = { "", "" };
    int count = sscanf(h->arg, "%15s %15s", to[0], to[1]);
    bridge_phase_name_t one = { "tick", to[0] };
    bridge_phase_name_t two[2] = { { "tock", to[0] }, { "tick", to[1] } };

    if (count == 2)
        load_mapped(h, two, 2);
    else if (count == 1)
        load_mapped(h, &one, 1);
    else
        load_mapped(h, nullptr, 0);
}
