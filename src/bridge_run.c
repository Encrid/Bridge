/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <string.h>

#include "bridge_inside.h"
#include "say.h"

static bool is_repair(const bridge_t *b, const char *repairing)
{
    size_t n = repairing ? bridge_get_stem_length(repairing) : 0;

    if (!repairing || !b->stopped_stem || strlen(b->stopped_stem) != n)
        return (false);
    return (!memcmp(b->stopped_stem, repairing, n));
}

int bridge_check_call(bridge_t *b, const char *repairing)
{
    const module_t *pinned = bridge_find_pinned(b);

    if (b->stopped_by && !is_repair(b, repairing)) {
        say_error(b, "the world has stopped: %s", b->stopped_by);
        return (1);
    }
    if (pinned) {
        say_error(b, "refused: module '%s' is pinned to the thread that "
            "loaded it, and this call comes from another", pinned->identity);
        return (1);
    }
    return (0);
}

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
