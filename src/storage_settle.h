/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#ifndef BRIDGE_SRC_STORAGE_SETTLE_H
    #define BRIDGE_SRC_STORAGE_SETTLE_H

    #include "storage_forget.h"

static inline void say_unapplied(bridge_t *b, module_t *m)
{
    say_t about = { m, nullptr, BRIDGE_ERR };

    if (m->left[0])
        say_about(b, &about, "module '%s': [budget] tables of %u is used "
            "up. Not applied, of the changes it asked: %u", m->identity,
            b->tables, m->left[0]);
    if (m->left[1])
        say_about(b, &about, "module '%s': [budget] columns of %u bytes is "
            "used up. Not applied, of the changes it asked: %u", m->identity,
            b->columns_len, m->left[1]);
    if (m->left[0] || m->left[1])
        m->fault_now = "by entities it changed that could not be placed";
    m->left[0] = 0;
    m->left[1] = 0;
}

static inline void say_barred(bridge_t *b, module_t *m)
{
    say_t about = { m, nullptr, BRIDGE_ERR };

    if (!m->barred)
        return;
    say_about(b, &about, "module '%s' asked to destroy entities that carry "
        "nothing it may write. Left as they were: %u", m->identity,
        m->barred);
    m->barred = 0;
    m->fault_now = "by destroys of entities that carry nothing it may "
        "write";
}

static inline void empty_queues(bridge_t *b)
{
    module_t *m = b->modules;

    while (m) {
        m->changed_now = m->queue_len != 0;
        m->queue_len = 0;
        m->mark_count = 0;
        m = m->next;
    }
}

static inline void settle_all(storage_t *r)
{
    module_t *m = r->b->modules;

    empty_queues(r->b);
    forget_all(r);
    while (m) {
        say_unapplied(r->b, m);
        say_barred(r->b, m);
        m = m->next;
    }
}

#endif
