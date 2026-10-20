/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <string.h>

#include "storage_publish.h"

static int write_list(publish_t *p, uint32_t q)
{
    module_t *m = p->m;
    const bridge_system_t *sys = m->observers[q];
    uint32_t queries = (uint32_t)sizeof(bridge_query_t) * m->system_count;
    bridge_list_t *slot = (bridge_list_t *)(m->priv + m->spans + queries) + q;
    bridge_trigger_t trigger = {};
    uint32_t k = 0;

    if (p->at + (uint64_t)sys->pending_count * sizeof(trigger) > p->end)
        return (1);
    slot->count = sys->pending_count;
    slot->at = (uint32_t)p->at;
    while (k < sys->pending_count) {
        trigger.id = sys->pending[k].id;
        trigger.events = sys->pending[k].events;
        memcpy(m->priv + p->at, &trigger, sizeof(trigger));
        p->at += sizeof(trigger);
        k++;
    }
    return (0);
}

int storage_publish_lists(publish_t *p)
{
    module_t *m = p->m;
    uint32_t pending = 0;
    uint32_t q = 0;

    while (q < m->observer_count) {
        pending |= m->observers[q]->pending_count;
        q++;
    }
    if (!pending && !m->had_triggers && !p->again)
        return (0);
    q = 0;
    while (q < m->observer_count && !write_list(p, q))
        q++;
    if (q < m->observer_count) {
        p->full = m->observers[q]->name;
        return (1);
    }
    m->had_triggers = pending != 0;
    return (0);
}

int storage_publish(storage_t *r, module_t *m, const char **list)
{
    publish_t p = { .r = r, .m = m, .end = (uint64_t)m->spans + m->spans_len };
    uint32_t tables = r->table_room + 1;
    uint32_t slots = (uint32_t)sizeof(bridge_query_t) * m->system_count
        + (uint32_t)sizeof(bridge_list_t) * m->observer_count;

    p.all = r->w->layout_moved;
    p.again = p.all;
    lay_private(&p, tables, slots);
    storage_publish_cells(&p);
    if (storage_publish_spans(&p) || storage_publish_lists(&p)) {
        *list = p.full;
        return (1);
    }
    return (0);
}

void storage_end_publish(storage_t *r)
{
    uint32_t i = 0;

    while (i < r->dirty_count) {
        r->tables[r->dirty[i]].touched = false;
        i++;
    }
    r->dirty_count = 0;
}

uint32_t storage_count_tables(storage_t *r)
{
    return (r->table_room + 1);
}
