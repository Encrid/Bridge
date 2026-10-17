/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "abi_slots.h"
#include "abi_subscribe.h"

int abi_check_before(subscribing_t *g)
{
    g->before = get_slots(g->m, g->before_at, g->before_count);
    if (!g->before)
        return (abi_refuse(g->m, "%s: before lies outside the scratch arena",
            g->what));
    if (g->before_count > g->slot)
        return (abi_refuse(g->m, "a%s %s states %u %ss before it, and the "
            "module registered %u", g->events ? "n" : "", g->what,
            g->before_count, g->what, g->slot));
    return (0);
}

static int read_before_slot(const subscribing_t *g, uint32_t i,
    uint32_t slot)
{
    const module_t *m = g->m;
    bridge_system_t *const *own = g->events ? m->observers : m->systems;

    if (slot >= g->slot)
        return (abi_refuse(g->m, "before %u of %s '%s' names slot %u, which "
            "is not an earlier %s", i, g->what, g->name, slot, g->what));
    if (own[slot]->phase != g->phase)
        return (abi_refuse(g->m, "before %u of system '%s' names system "
            "'%s', which is of another phase", i, g->name, own[slot]->name));
    schedule_join_before(g->s, own[slot]);
    return (0);
}

int abi_read_before(const subscribing_t *g)
{
    uint32_t i = 0;

    while (i < g->before_count
        && !read_before_slot(g, i, get_slot(g->before, i)))
        i++;
    return (i < g->before_count);
}
