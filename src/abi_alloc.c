/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge.
**
** All rights reserved.
**
** src/abi_alloc.c -- the persistent arena handed out, the marks of a
** module's queue, and a system taken out of storage for a reload.
*/

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "abi.h"

/*
** abi_refuse
**
** Refuses the call in progress, with why. The first refusal of a call is
** the reason, and is kept.
*/
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

/*
** abi_alloc
**
** alloc(len) -> offset into the persistent arena. Nothing frees it while its
** module is loaded. What is left of a range too small for this is dead
** space, as a string that outgrew its place leaves.
**
** strings.md, *It is given back when nothing points into it*
*/
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

/*
** abi_marks
**
** Makes room for a mark from each of a module's systems and observers, from
** the lifecycle call in progress, and `more` beyond those it has.
*/
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

/*
** abi_park
**
** Takes a system out of storage, for a reload.
*/
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

/*
** abi_unpark
**
** Puts a parked system back in storage, as it was: an observer with who is
** told, a system with its terms, and one with no term nowhere.
*/
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
