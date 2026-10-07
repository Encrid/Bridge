/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge.
**
** All rights reserved.
**
** src/block_barrier.c -- the barrier: what happens between two phases, in
** an order that never varies.
*/

#include "block.h"

static void refill_all(bridge_t *b)
{
    module_t *m = b->modules;

    while (m) {
        block_refill(b, m);
        m = m->next;
    }
}

/*
** publish_all
**
** Publishes every running module's spans, and stops one whose spans no
** longer hold what its queries match. A stopped module is published all
** over again, if it returns.
**
** budgets.md, *What running out does*
*/
static void publish_all(bridge_t *b)
{
    module_t *m = b->modules;

    while (m) {
        if (m->stopped)
            m->published = false;
        if (!m->stopped && storage_publish(b->storage, m)) {
            m->stopped = 1;
            say_about(b, &(say_t){ m, nullptr, BRIDGE_ERR }, "module '%s' "
                "is stopped: what its queries match no longer fits in its "
                "spans of %u bytes", m->identity, m->spans_len);
        }
        m = m->next;
    }
}

/*
** republish
**
** Publishes until publishing makes no more tables: a module's header says
** how many there are, so all are written again when that changed.
*/
static void republish(bridge_t *b)
{
    uint32_t tables = storage_count_tables(b->storage) + 1;

    while (tables != storage_count_tables(b->storage)) {
        tables = storage_count_tables(b->storage);
        publish_all(b);
    }
}

/*
** finish_barrier
**
** Ends a barrier once its observers have run: nobody is pending any more,
** the failure budgets are settled, and each module's log is drained.
**
** failure.md, *The budget*
*/
static void finish_barrier(bridge_t *b)
{
    module_t *m = b->modules;
    uint32_t i = 0;

    b->turns = 0;
    while (i < b->system_count) {
        b->systems[i]->pending_count = 0;
        i++;
    }
    schedule_account(b);
    while (m) {
        block_drain(b, m);
        m = m->next;
    }
    b->barriers++;
}

/*
** block_barrier
**
** The barrier. Queued changes are applied and spans republished; then the
** observers of what changed run, and what they ask for goes on their queues,
** and so is the next barrier's.
**
** schedule.md, *An observer runs at the barrier, one hop per barrier*
*/
void block_barrier(bridge_t *b)
{
    storage_apply(b->storage);
    refill_all(b);
    republish(b);
    storage_end_publish(b->storage);
    b->layout_moved = 0;
    schedule_run(b, b->phase_count);
    finish_barrier(b);
}
