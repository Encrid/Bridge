/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <string.h>

#include "module.h"

static uint32_t find_from(const bridge_phase_name_t *names, uint32_t i,
    const char *from)
{
    uint32_t at = 0;

    while (at < i && strcmp(names[at].from, from))
        at++;
    return (at);
}

static uint32_t find_to(const bridge_phase_name_t *names, uint32_t i,
    const char *to)
{
    uint32_t at = 0;

    while (at < i && strcmp(names[at].to, to))
        at++;
    return (at);
}

static int check_phase(const module_check_t *k, uint32_t i)
{
    const bridge_phase_name_t *names = k->cfg->phases;
    const bridge_phase_name_t *n = &names[i];
    uint32_t at;

    if (!n->from || !*n->from || !n->to || !*n->to)
        return (module_refuse(k->w, k->id, "phase name %u of its config is "
            "unset", i));
    if (find_from(names, i, n->from) < i)
        return (module_refuse(k->w, k->id, "its config maps phase '%s' "
            "twice", n->from));
    at = find_to(names, i, n->to);
    if (at < i)
        return (module_refuse(k->w, k->id, "its config maps phases '%s' and "
            "'%s' to one phase '%s'", names[at].from, n->from, n->to));
    if (bridge_find_phase(k->w->phases, k->w->phase_count, n->to,
        strlen(n->to)) < k->w->phase_count)
        return (0);
    return (module_refuse(k->w, k->id, "its config maps phase '%s' to '%s', "
        "which the deployment did not declare", n->from, n->to));
}

int module_check_phases(const module_check_t *k)
{
    int refused = 0;
    uint32_t i = 0;

    while (!refused && i < k->cfg->phase_count) {
        refused = check_phase(k, i);
        i++;
    }
    return (refused);
}
