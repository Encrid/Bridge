/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <string.h>

#include "bridge_inside.h"
#include "say.h"

int bridge_run_phase_now(bridge_world_t *w, const char *name)
{
    uint32_t phase;

    if (bridge_check_call(w, nullptr))
        return (1);
    phase = w->phase_count;
    if (name)
        phase = bridge_find_phase(w->phases, w->phase_count, name,
            strlen(name));
    if (phase == w->phase_count) {
        say_error(w, "phase '%s' is not one the deployment declared",
            name ? name : "");
        return (1);
    }
    schedule_run(w, phase);
    block_barrier(w);
    return (0);
}

int bridge_list_modules_now(bridge_world_t *w, bridge_module_info_t *out,
    int max)
{
    const module_t *m = w->modules;
    int count = 0;

    while (m) {
        if (out && count < max) {
            out[count].identity = m->identity;
            out[count].state = m->stopped ? BRIDGE_STOPPED : BRIDGE_RUNNING;
        }
        count++;
        m = m->next;
    }
    return (count);
}
