/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#ifndef BRIDGE_SRC_STORAGE_PUBLISH_H
    #define BRIDGE_SRC_STORAGE_PUBLISH_H

    #include "storage_inside.h"
    #include "storage_sets.h"

typedef struct {
    storage_t *r;
    module_t *m;
    uint32_t *cell;
    uint32_t *kept;
    uint32_t *stride;
    uint32_t *write;
    uint32_t *kind;
    uint32_t *owned;
    uint64_t at;
    uint64_t end;
    bool all;
    bool again;
    const char *full;
} publish_t;

static inline uint64_t span_bytes(uint32_t slots, uint32_t declared,
    uint32_t tables)
{
    uint64_t need = (uint64_t)slots + 4ull * declared * tables
        + 16ull * declared + 4ull + 8ull * BRIDGE_OWNED_CAP;

    return ((need + 7u) & ~7ull);
}

static inline void lay_private(publish_t *p, uint32_t tables, uint32_t slots)
{
    module_t *m = p->m;

    p->cell = (uint32_t *)(m->priv + m->spans + slots);
    p->kept = p->cell + (size_t)m->declared_count * tables;
    p->stride = p->kept + m->declared_count;
    p->write = p->stride + m->declared_count;
    p->kind = p->write + m->declared_count;
    p->owned = p->kind + m->declared_count;
    p->at = m->spans + span_bytes(slots, m->declared_count, tables);
}

static inline bool has_declared(const storage_t *r, const module_t *m,
    uint32_t t)
{
    const uint64_t *set = get_set(r, t);
    uint64_t met = 0;
    uint32_t w = 0;

    while (m->writable && w < r->words) {
        met |= set[w] & m->writable[r->words + w];
        w++;
    }
    return (met != 0);
}

void storage_publish_cells(publish_t *p);

[[nodiscard]] int storage_publish_spans(publish_t *p);

[[nodiscard]] int storage_publish_lists(publish_t *p);

#endif
