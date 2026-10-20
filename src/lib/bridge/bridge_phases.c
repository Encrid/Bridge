/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <string.h>

#include "bridge_inside.h"
#include "say.h"

static int add_phase(bridge_world_t *w, const bridge_config_t *cfg, uint32_t i)
{
    const char *name = cfg->phases[i];
    uint32_t k = 0;

    if (!name || !*name) {
        say_error(w, "phase %u has no name", i);
        return (1);
    }
    while (k < i && strcmp(w->phases[k], name))
        k++;
    if (k < i) {
        say_error(w, "phase '%s' is named twice", name);
        return (1);
    }
    w->phases[i] = copy_text(name, strlen(name));
    if (bridge_lacks(w, w->phases[i], "a phase's name"))
        return (1);
    w->phase_count = i + 1;
    return (0);
}

uint32_t bridge_find_phase(char *const *names, uint32_t count,
    const char *name, size_t len)
{
    uint32_t i = 0;

    while (i < count && (strlen(names[i]) != len
        || memcmp(names[i], name, len)))
        i++;
    return (i);
}

int bridge_name_phases(bridge_world_t *w, const bridge_config_t *cfg)
{
    uint32_t i = 0;

    if (!cfg->phase_count) {
        say_error(w, "the deployment declares no phase, and there is none "
            "of Bridge's to run");
        return (1);
    }
    while (i < cfg->phase_count && !add_phase(w, cfg, i))
        i++;
    return (i < cfg->phase_count);
}
