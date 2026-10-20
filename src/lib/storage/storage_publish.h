/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#ifndef BRIDGE_SRC_STORAGE_PUBLISH_H
    #define BRIDGE_SRC_STORAGE_PUBLISH_H

    #include "storage_inside.h"

typedef struct {
    storage_t *r;
    module_t *m;
    uint32_t *cell;
    uint32_t *kept;
    uint32_t *rows;
    uint32_t *blocks;
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

static inline void lay_private(publish_t *p, uint32_t tables, uint32_t slots)
{
    module_t *m = p->m;

    p->cell = (uint32_t *)(m->priv + m->spans + slots);
    p->kept = p->cell + (size_t)m->declared_count * tables;
    p->rows = p->kept + m->declared_count;
    p->blocks = p->rows + tables;
    p->stride = p->blocks + tables;
    p->write = p->stride + m->declared_count;
    p->kind = p->write + m->declared_count;
    p->owned = p->kind + m->declared_count;
    p->at = (uint64_t)((uint8_t *)(p->owned + 1 + 2u * BRIDGE_OWNED_CAP)
        - m->priv);
    p->at = (p->at + 7u) & ~7ull;
}

void storage_publish_cells(publish_t *p);

[[nodiscard]] int storage_publish_spans(publish_t *p);

[[nodiscard]] int storage_publish_lists(publish_t *p);

#endif
