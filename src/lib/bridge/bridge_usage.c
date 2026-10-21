/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "bridge_inside.h"

static uint32_t most_terms(const bridge_world_t *w)
{
    uint32_t most = 0;
    uint32_t i = 0;

    while (i < w->system_count) {
        if (w->systems[i]->terms > most)
            most = w->systems[i]->terms;
        i++;
    }
    return (most);
}

static void usage_fill(bridge_world_t *w, bridge_budget_t *b, bool peak)
{
    storage_usage_t su = storage_usage(w->storage);
    uint32_t sets = storage_count_free_sets(w->storage);
    uint32_t direct = storage_count_free_direct(w->storage);

    b->entities = peak ? su.entities_peak : su.entities;
    b->tables = peak ? su.tables_peak : su.tables;
    b->persistent = peak ? w->arena_space.peak : w->arena_space.used;
    b->columns = peak ? w->column_space.peak : w->column_space.used;
    b->types = w->type_count;
    b->systems = w->system_count;
    b->modules = w->module_count;
    b->terms = most_terms(w);
    b->sparse_sets = w->sparse_sets - sets;
    b->direct_indexed = w->direct_indexed - direct;
    b->pad = 0;
}

int bridge_usage_now(bridge_world_t *w, bridge_budget_t *used,
    bridge_budget_t *most)
{
    if (used)
        usage_fill(w, used, false);
    if (most)
        usage_fill(w, most, true);
    return (0);
}

int bridge_usage(bridge_world_t *w, bridge_budget_t *used,
    bridge_budget_t *most)
{
    bool owner;

    if (!bridge_enter(w, &owner))
        return (-1);
    (void)bridge_usage_now(w, used, most);
    bridge_leave(w, owner);
    return (0);
}
