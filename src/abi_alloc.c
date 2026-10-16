/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "abi.h"

int abi_refuse(module_t *m, const char *format, ...)
{
    va_list args;

    if (m->refused)
        return (1);
    va_start(args, format);
    vsnprintf(m->why, sizeof(m->why), format, args);
    va_end(args);
    m->refused = 1;
    return (1);
}

int abi_alloc(void *ud, const uint64_t *a, uint64_t *r)
{
    module_t *m = ud;
    uint64_t len = (a[0] + 7u) & ~7ull;

    if (m->persistent <= 0)
        return (abi_refuse(m, "alloc: it stated that it keeps nothing in "
            "the persistent arena"));
    while (len > m->arena_left && m->own_next < m->own_count) {
        m->arena_at = m->owned[m->own_next].at;
        m->arena_left = m->owned[m->own_next].len;
        m->own_next++;
    }
    if (len > m->arena_left)
        return (abi_refuse(m, "alloc: its persistent of %lld bytes is "
            "exhausted", (long long)m->persistent));
    r[0] = m->arena_at;
    m->arena_at += (uint32_t)len;
    m->arena_left -= (uint32_t)len;
    return (0);
}

bool abi_marks(module_t *m, uint32_t more)
{
    uint32_t want = m->mark_count + m->system_count + m->observer_count + 1;
    void *marks;

    want += more;
    if (want <= m->mark_room)
        return (true);
    marks = realloc(m->marks, want * sizeof(*m->marks));
    if (!marks)
        return (false);
    m->marks = marks;
    m->mark_room = want;
    return (true);
}

void abi_park(bridge_t *b, bridge_system_t *s)
{
    if (s->parked)
        return;
    if (s->matched)
        storage_forget(b->storage, s->matched);
    s->matched = nullptr;
    s->parked = 1;
    b->parked++;
}

void abi_unpark(bridge_t *b, bridge_system_t *s)
{
    storage_listener_t listener = { s->events, abi_hear, s };

    if (!s->parked)
        return;
    s->matched = nullptr;
    if (s->events)
        s->matched = storage_add_observer(b->storage, s->asked, s->terms,
            &listener);
    else if (s->terms)
        s->matched = storage_add_system(b->storage, s->asked, s->terms);
    s->parked = 0;
    b->parked--;
}
