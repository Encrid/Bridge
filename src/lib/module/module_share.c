/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "module.h"

uint64_t module_round_share(int64_t persistent)
{
    if (persistent <= 0)
        return (0);
    return (((uint64_t)persistent + 7u) & ~7ull);
}

void module_give_share(bridge_world_t *w, const range_t *owned,
    uint32_t own_count)
{
    for (uint32_t i = 0; i < own_count; i++)
        region_give(&w->arena_space, owned[i].at - w->arena, owned[i].len);
    w->arena_ranges -= own_count;
}

static uint64_t get_share_size(const module_t *m)
{
    uint64_t bytes = 0;
    uint32_t i = 0;

    while (i < m->own_count) {
        bytes += m->owned[i].len;
        i++;
    }
    return (bytes);
}

static bool is_share_held(bridge_world_t *w, const module_t *m)
{
    say_t about = { nullptr, nullptr, BRIDGE_WARN };
    unsigned long long bytes = get_share_size(m);
    bool held = false;
    uint32_t rows;
    uint32_t i = 0;

    while (i < w->type_count) {
        rows = module_count_pointing(w, m->owned, m->own_count, w->types[i]);
        if (rows)
            say_about(w, &about, "module '%s' is unloaded and its share of "
                "the persistent arena, %llu bytes, stays: '%s' holds a "
                "string in it, in this many rows: %u", m->identity, bytes,
                w->types[i]->name, rows);
        held = held || rows;
        i++;
    }
    return (held);
}

void module_settle_share(bridge_world_t *w, module_t *m)
{
    if (m->own_count && is_share_held(w, m))
        config_keep(w, m);
    else
        module_give_share(w, m->owned, m->own_count);
    config_give_back_free(w);
}
