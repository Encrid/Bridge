/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
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

void block_barrier(bridge_t *b)
{
    block_hand_out(b);
    storage_apply(b->storage);
    refill_all(b);
    publish_all(b);
    storage_end_publish(b->storage);
    b->layout_moved = 0;
    schedule_run(b, b->phase_count);
    finish_barrier(b);
}
