/*
** Copyright (C) 2026 Bridge Owner
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** src/abi_system.c -- the entries a module registers a system and an
** observer by, and what storage tells an observer.
**
** abi.md, *The ABI*
*/

#include <string.h>

#include "abi_entry.h"
#include "abi_subscribe.h"

/*
** abi_hear
**
** Storage says who and what; the observer itself runs at the barrier, after
** the republish. Its list is as long as what its module's spans could
** publish of it, and one past that is all that needs knowing:
** storage_publish finds it does not fit, and says so.
*/
void abi_hear(void *ctx, uint64_t id, enum bridge_event event)
{
    bridge_system_t *s = ctx;
    uint32_t n = s->pending_count;

    if (n > s->pending_room)
        return;
    if (n && s->pending[n - 1].id == id) {
        s->pending[n - 1].events |= event;
        return;
    }
    if (n < s->pending_room) {
        s->pending[n].id = id;
        s->pending[n].events = event;
    }
    s->pending_count++;
}

/*
** find_phase
**
** Answers which of `count` names is the `len` bytes at `at`, or `count`.
*/
static uint32_t find_phase(char *const *names, uint32_t count, const char *at,
    uint32_t len)
{
    uint32_t i = 0;

    while (i < count && (strlen(names[i]) != len || memcmp(names[i], at, len)))
        i++;
    return (i);
}

/*
** read_phase
**
** Finds the phase a system names: what the deployment says this module's
** name for a phase is, and otherwise the name.
**
** abi.md, *The ABI*
** schedule.md, *Bridge ships no phase names*
*/
static int read_phase(module_t *m, const bridge_system_statement_t *st,
    uint32_t *phase)
{
    const bridge_t *b = m->b;
    const char *at = get_scratch(m, st->phase, st->phase_len);
    uint32_t len = st->phase_len;
    uint32_t i;

    if (!at)
        return (abi_refuse(m, "add_system: a phase lies outside the scratch "
            "arena"));
    i = find_phase(m->phase_from, m->phase_names, at, len);
    if (i < m->phase_names) {
        *phase = m->phase_to[i];
        return (0);
    }
    *phase = find_phase(b->phases, b->phase_count, at, len);
    if (*phase < b->phase_count)
        return (0);
    return (abi_refuse(m, "a system names phase '%.*s', which the deployment "
        "did not declare", (int)len, at));
}

/*
** abi_add_system
**
** add_system(at) -> slot.
**
** abi.md, *The ABI*
*/
int abi_add_system(void *ud, const uint64_t *at, uint64_t *r)
{
    module_t *m = ud;
    bridge_system_statement_t st;
    const void *stated = get_scratch(m, at[0], sizeof(st));
    uint32_t phase;
    subscribing_t g;

    if (!stated)
        return (abi_refuse(m, "add_system: what it states lies outside the "
            "scratch arena"));
    memcpy(&st, stated, sizeof(st));
    if (!m->in_init)
        return (abi_refuse(m, "add_system is called outside init"));
    if (read_phase(m, &st, &phase))
        return (1);
    g = (subscribing_t){ m, st.name, st.name_len, st.terms, st.term_count,
        st.changes, st.change_count, phase, 0 };
    return (abi_subscribe(&g, r));
}

/*
** abi_observe
**
** observe(at) -> slot. An observer's phase is the one past the deployment's
** last: the barrier that applied its trigger.
**
** abi.md, *The ABI*
*/
int abi_observe(void *ud, const uint64_t *at, uint64_t *r)
{
    module_t *m = ud;
    bridge_observer_statement_t o;
    const void *stated = get_scratch(m, at[0], sizeof(o));
    uint32_t known = BRIDGE_ON_ENTER | BRIDGE_ON_ATTACH | BRIDGE_ON_LEAVE;
    subscribing_t g;

    if (!stated)
        return (abi_refuse(m, "observe: what it states lies outside the "
            "scratch arena"));
    memcpy(&o, stated, sizeof(o));
    if (!m->in_init)
        return (abi_refuse(m, "observe is called outside init"));
    if (!o.events || o.events & ~known)
        return (abi_refuse(m, "observe: its events are unset, or name one "
            "there is not"));
    g = (subscribing_t){ m, o.name, o.name_len, o.terms, o.term_count,
        o.changes, o.change_count, m->b->phase_count, o.events };
    return (abi_subscribe(&g, r));
}
