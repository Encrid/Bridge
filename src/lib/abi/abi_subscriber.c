/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <string.h>

#include "abi_slots.h"
#include "abi_subscribe.h"

static int check_budget(subscribing_t *g)
{
    const bridge_t *b = g->m->b;

    if (b->system_count - b->parked >= b->max_systems)
        return (abi_refuse(g->m, "it registers one more than [budget] "
            "systems of %u", b->max_systems));
    if (g->count > b->max_terms)
        return (abi_refuse(g->m, "a%s %s asks for %u terms, which is more "
            "than [budget] terms of %u", g->events ? "n" : "", g->what,
            g->count, b->max_terms));
    return (0);
}

static int read_stated(subscribing_t *g)
{
    module_t *m = g->m;
    const char *name = get_scratch(m, g->name_at, g->name_len);
    uint64_t terms_len = (uint64_t)g->count * sizeof(bridge_term_t);

    g->terms = get_scratch(m, g->terms_at, terms_len);
    g->changes = get_slots(m, g->changes_at, g->change_count);
    if (abi_check_name(m, g->what, g->name_len) || !name || !g->terms
        || !g->changes)
        return (abi_refuse(m, "%s: a name, a term or a change lies outside "
            "the scratch arena", g->what));
    if (g->change_count > m->declared_count)
        return (abi_refuse(m, "a%s %s states %u changes, and the module "
            "declared %u types", g->events ? "n" : "", g->what,
            g->change_count, m->declared_count));
    memcpy(g->name, name, g->name_len);
    g->name[g->name_len] = 0;
    return (0);
}

static bool is_taken(const module_t *m, const char *name)
{
    uint32_t i = 0;

    while (i < m->system_count && strcmp(m->systems[i]->name, name))
        i++;
    if (i < m->system_count)
        return (true);
    i = 0;
    while (i < m->observer_count && strcmp(m->observers[i]->name, name))
        i++;
    return (i < m->observer_count);
}

static int check_name(subscribing_t *g)
{
    const char *name = g->name;
    const char *mark = strpbrk(name, "@:");

    if (!strcmp(name, "init") || !strcmp(name, "shutdown")
        || !strcmp(name, "reload"))
        return (abi_refuse(g->m, "'%s' is a reserved name, and no %s may "
            "take it", name, g->what));
    if (mark)
        return (abi_refuse(g->m, "'%s' has '%c' in it, and no %s may have "
            "'@' or ':' in its name", name, *mark, g->what));
    if (is_taken(g->m, name))
        return (abi_refuse(g->m, "'%s' is registered twice", name));
    if (g->events && !g->count)
        return (abi_refuse(g->m, "observer '%s' has no term, and so nothing "
            "to observe", name));
    return (0);
}

int abi_check_subscriber(subscribing_t *g)
{
    return (check_budget(g) || read_stated(g) || check_name(g)
        || abi_check_before(g));
}
