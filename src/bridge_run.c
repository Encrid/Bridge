/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <string.h>

#include "bridge_inside.h"
#include "say.h"

static uint32_t find_phase(const bridge_t *b, const char *name)
{
    uint32_t phase = 0;

    if (!name)
        return (b->phase_count);
    while (phase < b->phase_count && strcmp(b->phases[phase], name))
        phase++;
    return (phase);
}

int bridge_run_phase(bridge_t *b, const char *name)
{
    uint32_t phase;

    if (bridge_check_call(b, nullptr))
        return (1);
    phase = find_phase(b, name);
    if (phase == b->phase_count) {
        say_error(b, "phase '%s' is not one the deployment declared",
            name ? name : "");
        return (1);
    }
    schedule_run(b, phase);
    block_barrier(b);
    return (0);
}

int bridge_list_modules(bridge_t *b, bridge_module_info_t *out, int max)
{
    const module_t *m = b->modules;
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
