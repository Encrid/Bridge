/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge-ASM700.
**
** All rights reserved.
**
** tests/stage5/stage5_phase.c -- a module that says "tick" for what this
** deployment calls "update".
**
** schedule.md, *A module's phase names are mapped by the deployment*
*/

#include "stage5.h"

static void load_ticking(host_t *h, const bridge_phase_name_t *tick)
{
    bridge_artifact_t a;

    if (host_open(h))
        return;
    host_state_native(&a, "game:stamp@0.1.0", NSTAMPTICK_PATH);
    a.cfg.spawns = 8;
    a.cfg.phases = tick;
    a.cfg.phase_count = tick ? 1 : 0;
    host_load(h, &a);
    stage5_load_watcher(h);
    host_run_frames_unsaid(h->b, 2);
    host_close(h);
}

/*
** stage5_leave_phase_unmapped
**
** Refused as it is: the deployment has no phase "tick".
*/
void stage5_leave_phase_unmapped(host_t *h)
{
    load_ticking(h, nullptr);
}

/*
** stage5_map_phase
**
** Loaded once the deployment says which of its phases "tick" is.
*/
void stage5_map_phase(host_t *h)
{
    bridge_phase_name_t tick = { "tick", "update" };

    load_ticking(h, &tick);
}

/*
** stage5_map_phase_nowhere
**
** Refused when the deployment maps "tick" to a phase it does not have.
*/
void stage5_map_phase_nowhere(host_t *h)
{
    bridge_phase_name_t tick = { "tick", "nowhere" };

    load_ticking(h, &tick);
}
