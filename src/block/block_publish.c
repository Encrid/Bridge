/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "block.h"

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

void block_publish_all(bridge_world_t *w)
{
    bool all = w->layout_moved || storage_is_dirty(w->storage);
    module_t *m;
    const char *list;

    block_sort_work(w);
    m = all ? w->modules : w->work;
    while (m) {
        list = nullptr;
        if (!m->stopped && storage_publish(w->storage, m, &list))
            stop_full(w, m, list);
        m = all ? m->next : m->next_work;
    }
}
