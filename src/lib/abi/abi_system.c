/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <string.h>

#include "abi_entry.h"
#include "abi_subscribe.h"

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
    if (abi_read_phase(m, &st, &phase))
        return (1);
    g = (subscribing_t){ m, st.name, st.name_len, st.terms, st.term_count,
        st.changes, st.change_count, st.before, st.before_count, phase, 0 };
    return (abi_subscribe(&g, r));
}

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
        o.changes, o.change_count, o.before, o.before_count,
        m->b->phase_count, o.events };
    return (abi_subscribe(&g, r));
}
