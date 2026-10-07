/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge-ASM700.
**
** All rights reserved.
**
** src/storage_spans.c -- a module's spans: for each of its systems, one span
** a table it matches.
*/

#include <string.h>

#include "storage_publish.h"
#include "storage_reach.h"
#include "storage_view.h"

/*
** write_span
**
** Writes one span: how many rows, where their ids are, and a base for each
** term. A term that must not match has no base.
*/
static void write_span(const storage_t *r, const storage_terms_t *s,
    uint32_t t, uint8_t *at)
{
    const table_t *tab = &r->tables[t];
    const uint32_t *columns = get_columns(r, t);
    uint32_t value = tab->rows;
    uint32_t i = 0;

    memset(at, 0, BRIDGE_SPAN_SIZE(s->count));
    memcpy(at, &value, 4);
    memcpy(at + 4, &tab->block, 4);
    while (i < s->count) {
        value = 0;
        if (s->oper[i] != BRIDGE_NOT)
            value = get_handed(r, s->type[i], columns[s->type[i]]);
        memcpy(at + BRIDGE_SPAN_BASE + 4u * i, &value, 4);
        i++;
    }
}

/*
** write_spans
**
** Writes every span of system `q` of a module, one after another, and where
** they begin. 1 if they do not fit.
*/
static int write_spans(publish_t *p, uint32_t q)
{
    module_t *m = p->m;
    storage_terms_t *s = m->systems[q]->matched;
    bridge_query_t *slot = (bridge_query_t *)(m->priv + m->spans) + q;
    uint32_t size = s ? BRIDGE_SPAN_SIZE(s->count) : 0;
    uint32_t i = 0;

    slot->spans = 0;
    slot->at = (uint32_t)p->at;
    if (!s)
        return (0);
    if (p->at + (uint64_t)s->matched * size > p->end)
        return (1);
    while (i < s->matched) {
        write_span(p->r, s, s->list[i], m->priv + p->at);
        p->at += size;
        i++;
    }
    slot->spans = s->matched;
    s->changed = false;
    return (0);
}

/*
** rewrite_spans
**
** Writes again, where it is, each span of system `q` whose table was
** touched.
*/
static void rewrite_spans(publish_t *p, uint32_t q)
{
    const storage_t *r = p->r;
    module_t *m = p->m;
    const storage_terms_t *s = m->systems[q]->matched;
    const bridge_query_t *slot = (bridge_query_t *)(m->priv + m->spans) + q;
    uint8_t *spans = m->priv + slot->at;
    uint32_t where;
    uint32_t i = 0;

    while (s && i < r->dirty_count) {
        where = s->pos[r->dirty[i]];
        if (where)
            write_span(r, s, r->dirty[i],
                spans + (where - 1) * BRIDGE_SPAN_SIZE(s->count));
        i++;
    }
}

/*
** write_every_span
**
** Writes all of a module's spans again, and where they end. A span is a
** multiple of four bytes, and what follows the spans holds an id, so the end
** is rounded to eight. 1 if they do not fit.
*/
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

/*
** storage_publish_spans
**
** Writes a module's spans: all of them again if any of its systems matches
** other tables than it did, and otherwise the ones whose table was touched.
*/
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
