/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** src/block_module.c -- what a module is given at a barrier: ids to hand
** out, and the header it reads its memory by.
*/

#include <string.h>

#include "block.h"

/*
** block_get_offset
**
** Answers where a pointer is in the shared memory, or 0.
*/
uint32_t block_get_offset(bridge_t *b, const void *p)
{
    const uint8_t *c = p;

    if (!c || c < b->shared || c >= b->shared + b->shared_len)
        return (0);
    return ((uint32_t)(c - b->shared));
}

/*
** block_refill
**
** Tops up a module's ids. Nothing used since the last barrier is nothing to
** move: the block is as long as its `spawns`, and moving it onto itself was
** most of a quiet barrier's cost. The index is twelve bytes times the
** highest id, and that is what was budgeted, so a refill can come up short.
*/
int block_refill(bridge_t *b, module_t *m)
{
    uint32_t want = m->spawns > 0 ? (uint32_t)m->spawns : 0;
    uint32_t kept = m->id_count - m->id_next;
    uint64_t *fresh = m->ids + kept;

    if (!want)
        return (0);
    if (m->id_next)
        memmove(m->ids, m->ids + m->id_next, kept * sizeof(*m->ids));
    m->id_next = 0;
    m->ids_short = 0;
    m->id_count = kept + storage_reserve(b->storage, fresh, want - kept);
    m->ids_short = m->id_count < want;
    return (m->ids_short);
}

/*
** block_write_header
**
** Writes what a module reads at offset 0 of its private memory: where its
** regions begin, and how many of each thing there are.
*/
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
