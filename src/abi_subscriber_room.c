/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "abi_subscribe.h"

static_assert(alignof(system_term_t) % alignof(uint64_t) == 0);
static_assert(sizeof(declared_type_t *) % alignof(uint64_t) == 0);

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

static bool make_room(subscribing_t *g)
{
    module_t *m = g->m;
    bridge_t *b = m->b;
    bridge_system_t ***mine = g->events ? &m->observers : &m->systems;
    size_t have = g->slot;
    size_t held = b->system_count;
    void *all = realloc(b->systems, (held + 1) * sizeof(*b->systems));
    void *own = realloc(*mine, (have + 1) * sizeof(**mine));

    if (all)
        b->systems = all;
    if (own)
        *mine = own;
    return (all && own);
}

static bool name_system(subscribing_t *g)
{
    const module_t *m = g->m;
    bridge_system_t *s = g->s;
    uint32_t room = m->spans_len / (uint32_t)sizeof(bridge_trigger_t);
    char full[256];

    snprintf(full, sizeof(full), "%s:%s", m->stem, g->name);
    s->name = copy_text(g->name, strlen(g->name));
    s->full = copy_text(full, strlen(full));
    s->named = schedule_find_name(&m->b->order, full);
    if (g->events) {
        s->pending_room = room;
        s->pending = calloc(room ? room : 1, sizeof(*s->pending));
    }
    return (s->name && s->full && (!g->events || s->pending));
}

int abi_alloc_subscriber(subscribing_t *g)
{
    bridge_system_t *s;
    size_t size = sizeof(*s) + g->count * sizeof(*s->term)
        + g->change_count * sizeof(*s->changes)
        + schedule_get_words(g->slot) * sizeof(*s->stated_before);
    bool had;

    s = calloc(1, size);
    g->s = s;
    g->asked = calloc(g->count + 1, sizeof(*g->asked));
    had = make_room(g);
    had = s && name_system(g) && had && g->asked;
    if (had && abi_marks(g->m, 1)) {
        s->changes = (declared_type_t **)(void *)&s->term[g->count];
        s->stated_before = (uint64_t *)(void *)&s->changes[g->change_count];
        return (0);
    }
    abi_free_subscriber(g);
    return (abi_refuse(g->m, "out of memory: there was none for %s '%s'",
        g->what, g->name));
}
