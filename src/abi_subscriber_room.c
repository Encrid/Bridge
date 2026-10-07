/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge-ASM700.
**
** All rights reserved.
**
** src/abi_subscriber_room.c -- the memory of a system or an observer:
** everything it needs is had before any of it is kept.
**
** failure.md, *Out of memory*
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "abi_subscribe.h"

/*
** abi_free_subscriber
**
** Frees what abi_alloc_subscriber allocated, of one that is refused.
*/
void abi_free_subscriber(subscribing_t *g)
{
    bridge_system_t *s = g->s;

    if (s) {
        free(s->name);
        free(s->full);
        free(s->pending);
    }
    free(s);
    free(g->asked);
    g->s = nullptr;
    g->asked = nullptr;
}

/*
** make_room
**
** Makes room for one more among the bridge's systems, and among the
** module's systems or its observers.
*/
static bool make_room(subscribing_t *g)
{
    module_t *m = g->m;
    bridge_t *b = m->b;
    bridge_system_t ***mine = g->events ? &m->observers : &m->systems;
    size_t have = g->events ? m->observer_count : m->system_count;
    size_t held = b->system_count;
    void *all = realloc(b->systems, (held + 1) * sizeof(*b->systems));
    void *own = realloc(*mine, (have + 1) * sizeof(**mine));

    if (all)
        b->systems = all;
    if (own)
        *mine = own;
    return (all && own);
}

/*
** name_system
**
** Gives a system its name, its name as [order] has it, and, an observer,
** its list of who triggered it: as long as what its module's spans could
** publish of it.
*/
static bool name_system(subscribing_t *g)
{
    const module_t *m = g->m;
    bridge_system_t *s = g->s;
    uint32_t room = m->spans_len / (uint32_t)sizeof(bridge_trigger_t);
    char full[256];

    snprintf(full, sizeof(full), "%s:%s", m->stem, g->name);
    s->name = copy_text(g->name, strlen(g->name));
    s->full = copy_text(full, strlen(full));
    if (g->events) {
        s->pending_room = room;
        s->pending = calloc(room ? room : 1, sizeof(*s->pending));
    }
    return (s->name && s->full && (!g->events || s->pending));
}

/*
** abi_alloc_subscriber
**
** Allocates everything a system or an observer needs. Its changes lie after
** its terms, in the same block.
**
** failure.md, *Out of memory*
*/
int abi_alloc_subscriber(subscribing_t *g)
{
    bridge_system_t *s;
    size_t size = sizeof(*s) + g->count * sizeof(*s->term)
        + g->change_count * sizeof(*s->changes);
    bool had;

    s = calloc(1, size);
    g->s = s;
    g->asked = calloc(g->count + 1, sizeof(*g->asked));
    had = make_room(g);
    had = s && name_system(g) && had && g->asked;
    if (had && abi_marks(g->m, 1)) {
        s->changes = (declared_type_t **)(void *)&s->term[g->count];
        return (0);
    }
    abi_free_subscriber(g);
    return (abi_refuse(g->m, "out of memory: there was none for %s '%s'",
        g->what, g->name));
}
