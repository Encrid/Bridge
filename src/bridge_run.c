/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge-ASM700.
**
** All rights reserved.
**
** src/bridge_run.c -- a phase run, and what is loaded said.
*/

#include <string.h>

#include "bridge_inside.h"
#include "say.h"

/*
** is_repair
**
** Answers whether a call is one a stopped bridge accepts: what stopped it
** can be brought back, or given up by an unload of it, and those are the
** calls that name it.
*/
static bool is_repair(const bridge_t *b, const char *repairing)
{
    size_t n = repairing ? bridge_get_stem_length(repairing) : 0;

    if (!repairing || !b->stopped_stem || strlen(b->stopped_stem) != n)
        return (false);
    return (!memcmp(b->stopped_stem, repairing, n));
}

/*
** bridge_check_call
**
** Answers 1, with a record, if nothing may run. Both calls that run module
** code come through here.
*/
int bridge_check_call(bridge_t *b, const char *repairing)
{
    const module_t *pinned = bridge_find_pinned(b);

    if (b->stopped_by && !is_repair(b, repairing)) {
        say_error(b, "the bridge has stopped: %s", b->stopped_by);
        return (1);
    }
    if (pinned) {
        say_error(b, "refused: module '%s' is pinned to the thread that "
            "loaded it, and this call comes from another", pinned->identity);
        return (1);
    }
    return (0);
}

/*
** find_phase
**
** Answers which phase of the deployment a name is, or how many there are if
** it is none of them.
*/
static uint32_t find_phase(const bridge_t *b, const char *name)
{
    uint32_t phase = 0;

    if (!name)
        return (b->phase_count);
    while (phase < b->phase_count && strcmp(b->phases[phase], name))
        phase++;
    return (phase);
}

/*
** bridge_run_phase
**
** Runs the systems registered to one phase, then its barrier. 0 if the phase
** ran, whatever its systems did.
*/
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

/*
** bridge_list_modules
**
** Says what is loaded, and whether it still runs. It is a pull, because a
** stop must not arrive by a route that may drop it.
**
** introspection.md, *Introspection*
*/
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
