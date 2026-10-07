/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** src/schedule_edges.c -- the systems of one phase, and what orders them:
** the deployment's pairs, and the order a module registered its own in.
*/

#include <string.h>

#include "schedule.h"

/*
** schedule_gather_phase
**
** Lists the systems of a phase that are not parked, with an empty matrix
** for them, in room made at create, [budget] systems wide.
*/
void schedule_gather_phase(bridge_t *b, schedule_phase_t *p)
{
    uint32_t i = 0;
    size_t words;

    p->n = 0;
    while (i < b->system_count) {
        if (b->systems[i]->phase == p->phase && !b->systems[i]->parked) {
            p->in[p->n] = b->systems[i];
            p->n++;
        }
        i++;
    }
    p->reach.n = p->n;
    p->reach.words = (p->n + 63) / 64;
    p->reach.bits = b->build_bits;
    words = (size_t)p->n * p->reach.words + 1;
    memset(p->reach.bits, 0, words * sizeof(*p->reach.bits));
}

/*
** find_system
**
** Answers which system of the phase has a full name, or how many there are
** if none has.
*/
static uint32_t find_system(const schedule_phase_t *p, const char *full)
{
    uint32_t i = 0;

    while (i < p->n && strcmp(p->in[i]->full, full))
        i++;
    return (i);
}

/*
** schedule_add_stated
**
** Adds the edges the deployment's pairs state between systems of the phase.
*/
void schedule_add_stated(const bridge_t *b, schedule_phase_t *p)
{
    uint32_t k = 0;
    uint32_t i;
    uint32_t j;

    while (k < b->order_count) {
        i = find_system(p, b->before[k]);
        j = find_system(p, b->after[k]);
        if (i < p->n && j < p->n)
            schedule_add_edge(&p->reach, i, j);
        k++;
    }
}

/*
** order_after
**
** Puts system `i` before each later system of its module that it conflicts
** with, unless that one already runs before it.
*/
static void order_after(schedule_phase_t *p, uint32_t i)
{
    const bridge_system_t *s = p->in[i];
    bool changing;
    uint32_t j = i + 1;

    while (j < p->n) {
        if (s->m == p->in[j]->m && !schedule_reaches(&p->reach, j, i)
            && schedule_find_conflict(s, p->in[j], &changing))
            schedule_add_edge(&p->reach, i, j);
        j++;
    }
}

/*
** schedule_add_own
**
** Adds the edges a module states by registering its systems in an order.
*/
void schedule_add_own(schedule_phase_t *p)
{
    uint32_t i = 0;

    while (i < p->n) {
        order_after(p, i);
        i++;
    }
}
