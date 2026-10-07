/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge-ASM700.
**
** All rights reserved.
**
** src/storage_terms.c -- a system's terms, and an observer's, handed over
** at load and forgotten at unload.
*/

#include <string.h>

#include "storage_table.h"

/*
** take_terms
**
** Takes one of the [budget] systems there are for a system's terms or an
** observer's, with its own part of every list. Whoever registers counted,
** so there is one.
*/
static storage_terms_t *take_terms(storage_t *r, const storage_term_t *terms,
    uint32_t count)
{
    uint32_t room = r->table_room + 2;
    uint32_t k = 0;
    storage_terms_t *t;

    while (r->terms_pool[k].used)
        k++;
    t = &r->terms_pool[k];
    *t = (storage_terms_t){ .count = count, .used = true };
    t->type = r->term_types + (size_t)k * r->b->max_terms;
    t->oper = r->term_opers + (size_t)k * r->b->max_terms;
    t->list = r->lists + (size_t)k * 2 * room;
    t->pos = t->list + room;
    memset(t->list, 0, 8u * room);
    while (count) {
        count--;
        t->type[count] = terms[count].type->id;
        t->oper[count] = terms[count].oper;
    }
    return (t);
}

/*
** storage_add_system
**
** Hands over a system's terms, and lists every table there is that it
** reads.
*/
storage_terms_t *storage_add_system(storage_t *r, const storage_term_t *terms,
    uint32_t count)
{
    storage_terms_t *s = take_terms(r, terms, count);
    uint32_t t = 1;

    while (t < r->table_high) {
        if (r->tables[t].live && terms_read(r, s, t)) {
            s->list[s->matched] = t;
            s->matched++;
            s->pos[t] = s->matched;
        }
        t++;
    }
    s->changed = true;
    r->systems[r->system_count] = s;
    r->system_count++;
    return (s);
}

/*
** storage_add_observer
**
** Hands over an observer's terms, and who is told what it is owed.
*/
storage_terms_t *storage_add_observer(storage_t *r,
    const storage_term_t *terms, uint32_t count,
    const storage_listener_t *listener)
{
    storage_terms_t *o = take_terms(r, terms, count);

    o->events = listener->events;
    o->heard = listener->heard;
    o->ctx = listener->ctx;
    r->observers[r->observer_count] = o;
    r->observer_count++;
    return (o);
}

/*
** storage_forget
**
** Forgets a system's terms, or an observer's. A system's place is taken by
** the last; an observer's by the ones after it, in order, since the order
** observers are told in is the order they came.
*/
void storage_forget(storage_t *r, storage_terms_t *terms)
{
    storage_terms_t **observers = r->observers;
    uint32_t i = 0;
    uint32_t k = 0;

    while (i < r->system_count && r->systems[i] != terms)
        i++;
    if (i < r->system_count) {
        r->system_count--;
        r->systems[i] = r->systems[r->system_count];
    }
    while (k < r->observer_count && observers[k] != terms)
        k++;
    if (k < r->observer_count) {
        r->observer_count--;
        memmove(&observers[k], &observers[k + 1],
            (r->observer_count - k) * sizeof(*observers));
    }
    terms->used = false;
}
