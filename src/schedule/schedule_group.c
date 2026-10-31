/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <string.h>

#include "bits.h"
#include "schedule.h"

static void seed_group(bridge_world_t *w, schedule_phase_t *p)
{
    bridge_system_t *s;

    p->n = 0;
    for (uint32_t i = 0; i < w->system_count; i++) {
        s = w->systems[i];
        if (s->phase == p->phase && s->dirty && !s->parked) {
            s->grouped = w->build_stamp;
            p->in[p->n] = s;
            p->n++;
        }
    }
}

static void add_neighbors(bridge_world_t *w, schedule_phase_t *p, uint32_t q)
{
    bridge_system_t *t;

    for (uint32_t i = 0; i < w->system_count; i++) {
        t = w->systems[i];
        if (t->grouped != w->build_stamp
            && schedule_is_neighbor(w, p->in[q], t)) {
            t->grouped = w->build_stamp;
            p->in[p->n] = t;
            p->n++;
        }
    }
}

static void order_group(bridge_world_t *w, schedule_phase_t *p)
{
    p->n = 0;
    for (uint32_t i = 0; i < w->system_count; i++) {
        if (w->systems[i]->grouped == w->build_stamp) {
            p->in[p->n] = w->systems[i];
            p->n++;
        }
    }
}

void schedule_gather_group(bridge_world_t *w, schedule_phase_t *p)
{
    size_t words;

    w->build_stamp++;
    seed_group(w, p);
    for (uint32_t q = 0; q < p->n; q++)
        add_neighbors(w, p, q);
    order_group(w, p);
    p->reach.n = p->n;
    p->reach.words = bits_words(p->n);
    p->reach.bits = w->build_bits;
    words = schedule_get_matrix_words(p->n);
    memset(p->reach.bits, 0, words * sizeof(*p->reach.bits));
}
