/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge-ASM700.
**
** All rights reserved.
**
** src/bridge_phases.c -- the phases of a deployment: each is a name, and no
** more.
*/

#include <string.h>

#include "bridge_inside.h"
#include "say.h"

/*
** add_phase
**
** Takes the phase `i` of the deployment. It is counted once it has its name,
** so that a bridge that is refused frees it.
*/
static int add_phase(bridge_t *b, const bridge_config_t *cfg, uint32_t i)
{
    const char *name = cfg->phases[i];
    uint32_t k = 0;

    if (!name || !*name) {
        say_error(b, "phase %u has no name", i);
        return (1);
    }
    while (k < i && strcmp(b->phases[k], name))
        k++;
    if (k < i) {
        say_error(b, "phase '%s' is named twice", name);
        return (1);
    }
    b->phases[i] = copy_text(name, strlen(name));
    if (bridge_lacks(b, b->phases[i], "a phase's name"))
        return (1);
    b->phase_count = i + 1;
    return (0);
}

/*
** bridge_name_phases
**
** Takes the deployment's phases. It declares its own: there is none of
** Bridge-ASM700's to run.
*/
int bridge_name_phases(bridge_t *b, const bridge_config_t *cfg)
{
    uint32_t i = 0;

    if (!cfg->phase_count) {
        say_error(b, "the deployment declares no phase, and there is none "
            "of Bridge-ASM700's to run");
        return (1);
    }
    while (i < cfg->phase_count && !add_phase(b, cfg, i))
        i++;
    return (i < cfg->phase_count);
}
