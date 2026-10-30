/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "block.h"

static void refill_all(bridge_world_t *w)
{
    module_t *m = w->modules;

    while (m) {
        block_refill(w, m);
        m = m->next;
    }
}

static void finish_barrier(bridge_world_t *w)
{
    module_t *m = w->modules;
    uint32_t i = 0;

    w->turns = 0;
    while (i < w->system_count) {
        w->systems[i]->pending_count = 0;
        i++;
    }
    schedule_account(w);
    while (m) {
        block_drain(w, m);
        m = m->next;
    }
    w->barriers++;
}

static void run_barrier(bridge_world_t *w)
{
    block_hand_out(w);
    storage_apply(w->storage);
    refill_all(w);
    block_publish_all(w);
    storage_end_publish(w->storage);
    if (w->validate)
        storage_validate_baseline(w->storage);
    w->layout_moved = 0;
    schedule_run(w, w->phase_count);
    finish_barrier(w);
}

static bool has_queued(const bridge_world_t *w)
{
    const module_t *m = w->modules;

    while (m && !m->queue_len)
        m = m->next;
    return (m != nullptr);
}

void block_barrier(bridge_world_t *w)
{
    uint32_t passes = 0;

    run_barrier(w);
    passes++;
    while (passes < w->observer_passes && has_queued(w)) {
        run_barrier(w);
        passes++;
    }
}
