/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "schedule.h"

static void weigh(const bridge_system_t **best, const bridge_system_t *s,
    const bridge_system_t *t)
{
    const bridge_system_t *a = s->at < t->at ? s : t;
    const bridge_system_t *b = s->at < t->at ? t : s;

    if (!best[0] || a->at < best[0]->at
        || (a->at == best[0]->at && b->at < best[1]->at)) {
        best[0] = a;
        best[1] = b;
    }
}

static void check_list(const schedule_phase_t *p, const bridge_system_t *s,
    const system_use_t *u, const bridge_system_t **best)
{
    const bridge_system_t *t;

    while (u) {
        t = u->s;
        if (t != s && t->phase == s->phase && !t->parked
            && schedule_is_unordered(p->order, s, t))
            weigh(best, s, t);
        u = u->next;
    }
}

bool schedule_find_unordered(const schedule_phase_t *p,
    const bridge_system_t **pair)
{
    const bridge_system_t *s;

    pair[0] = nullptr;
    pair[1] = nullptr;
    for (uint32_t i = 0; i < p->n; i++) {
        s = p->in[i];
        for (uint32_t k = 0; s->fresh && k < s->use_count; k++)
            check_list(p, s, s->use[k].type->uses, pair);
    }
    return (pair[0] != nullptr);
}
