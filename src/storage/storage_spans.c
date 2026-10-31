/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <string.h>

#include "storage_publish.h"

static int write_spans(publish_t *p, uint32_t q)
{
    module_t *m = p->m;
    storage_terms_t *s = m->systems[q]->matched;
    bridge_query_t *slot = (bridge_query_t *)(m->priv + m->spans) + q;
    uint64_t size = s ? (uint64_t)s->matched * BRIDGE_SPAN_SIZE : 0;

    slot->spans = 0;
    slot->at = (uint32_t)p->at;
    if (!s)
        return (0);
    if (p->at + size > p->end)
        return (1);
    memcpy(m->priv + p->at, s->list, (size_t)size);
    p->at += size;
    slot->spans = s->matched;
    s->changed = false;
    return (0);
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
    p->at = m->spans_end;
    return (0);
}
