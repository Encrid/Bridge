/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "host.h"

static bool take_phases(host_t *h)
{
    static const char *const PHASES[] = { "phases" };
    const value_t *v = config_find_key(&h->config, PHASES, 1).value;

    if (!v)
        return (false);
    h->phases = malloc(sizeof(*h->phases) * (v->count + 1));
    for (uint32_t i = 0; h->phases && i < v->count; i++)
        h->phases[i] = v->items[i].text;
    h->phase_count = v->count;
    return (h->phases != nullptr);
}

static bool take_run(host_t *h)
{
    static const char *const THREADS[] = { "threads" };
    static const char *const ROUNDS[] = { "rounds" };
    static const char *const LIVE[] = { "live" };
    const value_t *threads = config_find_key(&h->config, THREADS, 1).value;
    const value_t *rounds = config_find_key(&h->config, ROUNDS, 1).value;
    const value_t *live = config_find_key(&h->config, LIVE, 1).value;

    if (threads)
        h->threads = (uint32_t)threads->whole;
    if (rounds)
        h->rounds = (int)rounds->whole;
    if (live)
        h->live = live->flag;
    return (threads && rounds && live);
}

static bool take_field(host_t *h, const char *field, size_t at, bool wide)
{
    const char *const names[] = { "budget", field };
    const value_t *v = config_find_key(&h->config, names, 2).value;
    int64_t n = v && v->kind == VALUE_WHOLE ? v->whole : BRIDGE_NONE;
    uint32_t narrow = (uint32_t)n;

    if (v && wide)
        memcpy((char *)&h->budget + at, &n, 8);
    else if (v)
        memcpy((char *)&h->budget + at, &narrow, 4);
    return (v != nullptr);
}

static bool take_budget(host_t *h)
{
    static const char *const NAME[] = { "entities", "tables", "types",
        "systems", "terms", "modules", "persistent", "columns", "sparse_sets",
        "direct_indexed" };
    static const size_t AT[] = { offsetof(bridge_budget_t, entities),
        offsetof(bridge_budget_t, tables), offsetof(bridge_budget_t, types),
        offsetof(bridge_budget_t, systems), offsetof(bridge_budget_t, terms),
        offsetof(bridge_budget_t, modules),
        offsetof(bridge_budget_t, persistent),
        offsetof(bridge_budget_t, columns),
        offsetof(bridge_budget_t, sparse_sets),
        offsetof(bridge_budget_t, direct_indexed) };
    bool taken = true;

    for (uint32_t i = 0; i < 10; i++)
        taken = take_field(h, NAME[i], AT[i], !i || i >= 6) && taken;
    return (taken);
}

bool config_world(host_t *h)
{
    bool taken = take_phases(h);

    taken = take_run(h) && taken;
    taken = take_budget(h) && taken;
    take_checks(h);
    return (config_order(h) && taken);
}
