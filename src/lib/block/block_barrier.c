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

static void stop_full(bridge_world_t *w, module_t *m, const char *list)
{
    const say_t about = { m, nullptr, BRIDGE_ERR };

    m->stopped = 1;
    if (list) {
        say_about(w, &about, "module '%s' is stopped: the list of its "
            "observer '%s' no longer fits in its spans of %u bytes",
            m->identity, list, m->spans_len);
    } else {
        say_about(w, &about, "module '%s' is stopped: what its queries "
            "match no longer fits in its spans of %u bytes", m->identity,
            m->spans_len);
    }
    if (m->on_error == BRIDGE_FAIL)
        bridge_stop(w, m->identity, "was stopped");
}

static void publish_all(bridge_world_t *w)
{
    module_t *m = w->modules;
    const char *list;

    while (m) {
        list = nullptr;
        if (!m->stopped && storage_publish(w->storage, m, &list))
            stop_full(w, m, list);
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

void block_barrier(bridge_world_t *w)
{
    block_hand_out(w);
    storage_apply(w->storage);
    refill_all(w);
    publish_all(w);
    storage_end_publish(w->storage);
    w->layout_moved = 0;
    schedule_run(w, w->phase_count);
    finish_barrier(w);
}
