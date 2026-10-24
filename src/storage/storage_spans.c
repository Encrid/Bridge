/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <string.h>

#include "storage_publish.h"
#include "storage_reach.h"
#include "storage_view.h"

static void write_span(const storage_t *r, uint32_t t, uint8_t *at)
{
    const table_t *tab = &r->tables[t];

    memcpy(at, &t, 4);
    memcpy(at + 4, &tab->rows, 4);
}

static int write_spans(publish_t *p, uint32_t q)
{
    module_t *m = p->m;
    storage_terms_t *s = m->systems[q]->matched;
    bridge_query_t *slot = (bridge_query_t *)(m->priv + m->spans) + q;
    uint32_t size = s ? BRIDGE_SPAN_SIZE : 0;

    slot->spans = 0;
    slot->at = (uint32_t)p->at;
    m->systems[q]->spans_at = (uint32_t)p->at;
    if (!s)
        return (0);
    if (p->at + (uint64_t)s->matched * size > p->end)
        return (1);
    for (uint32_t i = 0; i < s->matched; i++) {
        write_span(p->r, s->list[i], m->priv + p->at);
        p->at += size;
    }
    slot->spans = s->matched;
    s->changed = false;
    return (0);
}

static void rewrite_spans(publish_t *p, uint32_t q)
{
    const storage_t *r = p->r;
    module_t *m = p->m;
    const storage_terms_t *s = m->systems[q]->matched;
    uint8_t *spans = m->priv + m->systems[q]->spans_at;
    uint32_t where;
    uint32_t i = 0;

    while (s && i < r->dirty_count) {
        where = s->pos[r->dirty[i]];
        if (where)
            write_span(r, r->dirty[i],
                spans + (where - 1) * BRIDGE_SPAN_SIZE);
        i++;
    }
}

static int write_every_span(publish_t *p)
{
    module_t *m = p->m;
    uint32_t q = 0;

    while (q < m->system_count) {
        if (write_spans(p, q))
            return (1);
        q++;
    }
    p->at = (p->at + 7u) & ~7ull;
    if (p->at > p->end)
        return (1);
    m->spans_end = (uint32_t)p->at;
    return (0);
}

int storage_publish_spans(publish_t *p)
{
    module_t *m = p->m;
    uint32_t q = 0;

    while (q < m->system_count) {
        if (m->systems[q]->matched && m->systems[q]->matched->changed)
            p->again = true;
        q++;
    }
    if (p->again)
        return (write_every_span(p));
    q = 0;
    while (q < m->system_count) {
        rewrite_spans(p, q);
        q++;
    }
    p->at = m->spans_end;
    return (0);
}
