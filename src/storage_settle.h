/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** src/storage_settle.h -- the end of a barrier's work on rows: each module's
** queue emptied, and each told what of what it asked was not applied.
*/

#ifndef BRIDGE_SRC_STORAGE_SETTLE_H
    #define BRIDGE_SRC_STORAGE_SETTLE_H

    #include "storage_forget.h"

/*
** say_unapplied
**
** Tells a module how many of the changes it asked were not applied for want
** of a table, and how many for want of columns. Either counts toward its
** failure budget.
**
** failure.md, *The budget*
*/
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
        m->unplaced_now = 1;
    m->left[0] = 0;
    m->left[1] = 0;
}

/*
** say_barred
**
** Tells a module how many entities it asked to destroy that carry nothing
** it may write, and were left as they were.
*/
static inline void say_barred(bridge_t *b, module_t *m)
{
    say_t about = { m, nullptr, BRIDGE_ERR };

    if (!m->barred)
        return;
    say_about(b, &about, "module '%s' asked to destroy entities that carry "
        "nothing it may write. Left as they were: %u", m->identity,
        m->barred);
    m->barred = 0;
    m->unplaced_now = 1;
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

/*
** settle_all
**
** Ends a barrier's work on rows: the queues are empty, the fold is
** forgotten, and each module is told what was not applied.
*/
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
