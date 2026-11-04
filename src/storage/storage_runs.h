/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#ifndef BRIDGE_SRC_STORAGE_RUNS_H
    #define BRIDGE_SRC_STORAGE_RUNS_H

    #include "storage_inside.h"

static inline bool is_before(const module_t *a, const module_t *b)
{
    uint64_t turn = a->marks[a->mark_at].turn;
    uint64_t other = b->marks[b->mark_at].turn;

    if (turn != other)
        return (turn < other);
    return (a->place < b->place);
}

static inline void sink_run(storage_t *r, uint32_t at)
{
    module_t *m = r->heap[at];
    uint32_t below = 2 * at + 1;

    while (below < r->heap_count) {
        if (below + 1 < r->heap_count
            && is_before(r->heap[below + 1], r->heap[below]))
            below++;
        if (is_before(r->heap[below], m)) {
            r->heap[at] = r->heap[below];
            at = below;
            below = 2 * at + 1;
        } else {
            below = r->heap_count;
        }
    }
    r->heap[at] = m;
}

static inline void reset_marks(storage_t *r)
{
    module_t *m = r->w->work;

    r->heap_count = 0;
    while (m) {
        m->mark_at = 0;
        if (m->mark_count) {
            r->heap[r->heap_count] = m;
            r->heap_count++;
        }
        m = m->next_work;
    }
    for (uint32_t at = r->heap_count / 2; at > 0; at--)
        sink_run(r, at - 1);
}

static inline module_t *take_next_run(storage_t *r, uint32_t *from,
    uint32_t *to)
{
    module_t *first;

    if (!r->heap_count)
        return (nullptr);
    first = r->heap[0];
    *from = first->marks[first->mark_at].at;
    first->mark_at++;
    if (first->mark_at < first->mark_count) {
        *to = first->marks[first->mark_at].at;
    } else {
        *to = first->queue_len;
        r->heap_count--;
        r->heap[0] = r->heap[r->heap_count];
    }
    if (r->heap_count)
        sink_run(r, 0);
    return (first);
}

#endif
