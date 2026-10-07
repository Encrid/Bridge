/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** src/storage_runs.h -- the queues, taken a run at a time in the order the
** calls that filled them ran, and the values they carry written.
**
** threads.md, *Threads*
*/

#ifndef BRIDGE_SRC_STORAGE_RUNS_H
    #define BRIDGE_SRC_STORAGE_RUNS_H

    #include "storage_fold.h"
    #include "storage_value.h"

static inline void reset_marks(bridge_t *b)
{
    module_t *m = b->modules;

    while (m) {
        m->mark_at = 0;
        m = m->next;
    }
}

/*
** find_next_run
**
** Answers the module whose next run comes first, or nullptr when every
** queue has been taken whole. The queues are applied in the order the calls
** that filled them ran, and not a module at a time; a module's own queue is
** in that order already, so this is the earliest of what each has next.
*/
static inline module_t *find_next_run(bridge_t *b)
{
    module_t *first = nullptr;
    module_t *m = b->modules;

    while (m) {
        if (m->mark_at != m->mark_count && (!first
            || m->marks[m->mark_at].turn < first->marks[first->mark_at].turn))
            first = m;
        m = m->next;
    }
    return (first);
}

/*
** take_next_run
**
** Takes the next run of changes: one module's, of one turn. Answers the
** module and writes where in its queue the run begins and ends, or nullptr
** when there is none left.
*/
static inline module_t *take_next_run(bridge_t *b, uint32_t *from,
    uint32_t *to)
{
    module_t *first = find_next_run(b);

    if (!first)
        return (nullptr);
    *from = first->marks[first->mark_at].at;
    first->mark_at++;
    *to = first->queue_len;
    if (first->mark_at < first->mark_count)
        *to = first->marks[first->mark_at].at;
    return (first);
}

/*
** is_written
**
** Answers whether a change is an attach whose value is to be written: of an
** entity this barrier changed and neither destroyed nor left as it was, and
** that is the entity the change named.
*/
[[gnu::always_inline]] static inline bool is_written(const storage_t *r,
    const change_t *ch)
{
    uint32_t index = (uint32_t)ch->id;
    uint8_t folded = index < r->high ? r->folded[index] : 0;

    if (ch->op != CHANGE_ATTACH || !(folded & TOUCHED))
        return (false);
    if (folded & (DESTROYED | DROPPED))
        return (false);
    return (r->places[index].generation == (uint32_t)(ch->id >> 32));
}

/*
** write_value
**
** Writes the value one change carries, and counts for its module a change
** of an entity that was left as it was: a change that was not applied is
** told to who asked it.
**
** budgets.md, *What running out does*
*/
[[gnu::always_inline]] static inline void write_value(storage_t *r,
    module_t *m, const change_t *ch, const uint8_t *value)
{
    uint32_t index = (uint32_t)ch->id;
    uint8_t folded = index < r->high ? r->folded[index] : 0;
    uint32_t to = 0;

    if (folded & DROPPED
        && r->places[index].generation == (uint32_t)(ch->id >> 32))
        m->left[(folded & SHORT) != 0]++;
    if (is_written(r, ch))
        to = get_value(r, index, ch->c->type);
    if (to)
        copy_bytes(r->b->shared + to, value, ch->size);
}

#endif
