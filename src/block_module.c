/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <string.h>

#include "block.h"

uint32_t block_get_offset(bridge_t *b, const void *p)
{
    const uint8_t *c = p;

    if (!c || c < b->shared || c >= b->shared + b->shared_len)
        return (0);
    return ((uint32_t)(c - b->shared));
}

int block_refill(bridge_t *b, module_t *m)
{
    uint32_t want = m->spawns > 0 ? (uint32_t)m->spawns : 0;
    uint32_t kept = m->id_count - m->id_next;

    if (kept > want) {
        storage_release(b->storage, m->ids + m->id_next + want, kept - want);
        kept = want;
    }
    if (m->id_next)
        memmove(m->ids, m->ids + m->id_next, kept * sizeof(*m->ids));
    m->id_next = 0;
    m->id_count = kept;
    if (!want)
        return (0);
    m->id_count += storage_reserve(b->storage, m->ids + kept, want - kept);
    m->ids_short = m->id_count < want;
    return (m->ids_short);
}

void block_hand_out(bridge_t *b)
{
    module_t *m = b->modules;

    while (m) {
        storage_hand_out(b->storage, m->ids, m->id_next);
        if (m->lender)
            storage_hand_out(b->storage, m->lender->ids, m->lender->id_next);
        m = m->next;
    }
}

void block_write_header(bridge_t *b, module_t *m)
{
    bridge_header_t *h = (bridge_header_t *)m->priv;
    uint32_t queries = (uint32_t)sizeof(bridge_query_t) * m->system_count;
    uint32_t lists = (uint32_t)sizeof(bridge_list_t) * m->observer_count;

    h->abi = BRIDGE_ABI;
    h->scratch = m->scratch;
    h->scratch_len = m->scratch_len;
    h->ring = m->ring;
    h->ring_len = m->ring_len;
    h->ring_tail = m->ring_tail;
    h->queries = m->spans;
    h->observers = m->spans + queries;
    h->column_base = h->observers + lists;
    h->types = m->declared_count;
    h->tables = storage_count_tables(b->storage);
    h->kept = h->column_base + 4u * h->types * h->tables;
    h->entity_index = block_get_offset(b, b->index);
    h->entities = b->entities;
}
