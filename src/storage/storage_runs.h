/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#ifndef BRIDGE_SRC_STORAGE_RUNS_H
    #define BRIDGE_SRC_STORAGE_RUNS_H

    #include "storage_inside.h"

static inline void reset_marks(bridge_world_t *w)
{
    module_t *m = w->work;

    while (m) {
        m->mark_at = 0;
        m = m->next_work;
    }
}

static inline module_t *find_next_run(bridge_world_t *w)
{
    module_t *first = nullptr;
    module_t *m = w->work;

    while (m) {
        if (m->mark_at != m->mark_count && (!first
            || m->marks[m->mark_at].turn < first->marks[first->mark_at].turn))
            first = m;
        m = m->next_work;
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

#endif
