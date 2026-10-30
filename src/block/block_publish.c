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
    module_t *m = w->modules;
    const char *list;

    while (m) {
        list = nullptr;
        if (!m->stopped && storage_publish(w->storage, m, &list))
            stop_full(w, m, list);
        m = m->next;
    }
}
