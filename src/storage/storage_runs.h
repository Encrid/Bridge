/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#ifndef BRIDGE_SRC_STORAGE_RUNS_H
    #define BRIDGE_SRC_STORAGE_RUNS_H

    #include "storage_fold.h"
    #include "storage_value.h"

static inline void reset_marks(bridge_world_t *w)
{
    module_t *m = w->modules;

    while (m) {
        m->mark_at = 0;
        m = m->next;
    }
}

static inline module_t *find_next_run(bridge_world_t *w)
{
    module_t *first = nullptr;
    module_t *m = w->modules;

    while (m) {
        if (m->mark_at != m->mark_count && (!first
            || m->marks[m->mark_at].turn < first->marks[first->mark_at].turn))
            first = m;
        m = m->next;
    }
    return (first);
}

static inline module_t *take_next_run(bridge_world_t *w, uint32_t *from,
    uint32_t *to)
{
    module_t *first = find_next_run(w);

    if (!first)
        return (nullptr);
    *from = first->marks[first->mark_at].at;
    first->mark_at++;
    *to = first->queue_len;
    if (first->mark_at < first->mark_count)
        *to = first->marks[first->mark_at].at;
    return (first);
}

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
        copy_bytes(r->w->shared + to, value, ch->size);
}

#endif
