/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge-ASM700.
**
** All rights reserved.
**
** src/abi_subscribe.c -- a system or an observer registered.
*/

#include <stdio.h>
#include <string.h>

#include "abi_subscribe.h"

/*
** read_changes
**
** Reads what a system or an observer attaches or detaches: a write, so it
** wants the grant a write wants. A call that was refused before a change is
** refused at it.
**
** abi.md, *The ABI*
** grants.md, *Whose component it is*
*/
static int read_changes(subscribing_t *g)
{
    module_t *m = g->m;
    bridge_system_t *s = g->s;
    const char *at = g->changes;
    uint32_t slot;
    uint32_t i = 0;

    while (i < g->change_count) {
        memcpy(&slot, at + i * sizeof(slot), sizeof(slot));
        if (slot >= m->declared_count)
            abi_refuse(m, "change %u of %s '%s' names slot %u, which it "
                "never declared", i, g->what, g->name, slot);
        if (!m->refused && !abi_is_writable(m, m->declared[slot]))
            abi_refuse(m, "%s '%s' may not attach or detach '%s': no grant "
                "says so", g->what, g->name, m->declared[slot]->name);
        if (m->refused)
            return (1);
        s->changes[s->change_count++] = m->declared[slot];
        i++;
    }
    return (0);
}

/*
** read_term
**
** Reads term `i`: what storage is asked, and what the conflict graph reads
** of it. A term that must not match touches nothing.
*/
static int read_term(subscribing_t *g, uint32_t i)
{
    bridge_system_t *s = g->s;
    declared_type_t *c;
    bridge_term_t t;

    memcpy(&t, &g->terms[i], sizeof(t));
    snprintf(g->of, sizeof(g->of), "term %u of %s '%s'", i, g->what, g->name);
    if (abi_check_term(g, i, &t))
        return (1);
    c = g->m->declared[t.slot];
    g->asked[i].type = c->type;
    g->asked[i].oper = t.oper;
    s->term[i].touches = t.oper == BRIDGE_NOT ? nullptr : c;
    s->term[i].writes = t.access == BRIDGE_WRITE;
    return (0);
}

/*
** hand_over
**
** Hands a system's or an observer's terms to storage. A system with no term
** matches nothing, and storage holds nothing of it.
*/
static void hand_over(subscribing_t *g)
{
    storage_t *storage = g->m->b->storage;
    bridge_system_t *s = g->s;
    storage_listener_t hears = { g->events, abi_hear, s };

    if (g->events)
        s->matched = storage_add_observer(storage, g->asked, g->count, &hears);
    if (!g->events && g->count)
        s->matched = storage_add_system(storage, g->asked, g->count);
    s->asked = g->asked;
    s->events = g->events;
    s->m = g->m;
    s->phase = g->phase;
    s->terms = g->count;
    s->observer = g->events != 0;
}

/*
** list_system
**
** Puts a system among the bridge's, and among its module's systems or its
** observers, and answers its slot there.
*/
static uint32_t list_system(subscribing_t *g)
{
    module_t *m = g->m;
    bridge_t *b = m->b;

    b->systems[b->system_count++] = g->s;
    if (g->events) {
        m->observers[m->observer_count] = g->s;
        return (m->observer_count++);
    }
    m->systems[m->system_count] = g->s;
    return (m->system_count++);
}

/*
** abi_subscribe
**
** Registers a system or an observer, and answers its slot in `r`.
*/
int abi_subscribe(subscribing_t *g, uint64_t *r)
{
    int refused;
    uint32_t i = 0;

    g->what = g->events ? "observer" : "system";
    if (abi_check_subscriber(g) || abi_alloc_subscriber(g))
        return (1);
    refused = read_changes(g);
    while (!refused && i < g->count) {
        refused = read_term(g, i);
        i++;
    }
    if (refused) {
        abi_free_subscriber(g);
        return (1);
    }
    hand_over(g);
    r[0] = list_system(g);
    return (0);
}
