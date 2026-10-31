/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdlib.h>

#include "storage_set.h"

uint32_t storage_count_free_sets(const storage_t *r)
{
    uint32_t n = 0;
    uint32_t slot = 0;

    while (slot < r->set_room) {
        n += !r->set_type[slot];
        slot++;
    }
    return (n);
}

uint32_t storage_count_free_direct(const storage_t *r)
{
    uint32_t n = 0;
    uint32_t slot = 0;

    while (slot < r->direct_room) {
        n += !r->direct_type[slot];
        slot++;
    }
    return (n);
}

bool storage_make_writable(storage_t *r, module_t *m)
{
    if (!m->writable)
        m->writable = calloc(2 * (size_t)r->words, sizeof(*m->writable));
    return (m->writable != nullptr);
}

void storage_let_use(const storage_t *r, module_t *m,
    const storage_type_t *type, bool write)
{
    set_put(m->writable + r->words, type->id);
    if (write)
        set_put(m->writable, type->id);
}

void storage_undeclare(storage_t *r, storage_type_t *type)
{
    bridge_world_t *w = r->w;

    r->apart -= type->kept != BRIDGE_TABLE && type->kept != BRIDGE_TOGGLE;
    r->tags -= !type->size;
    if (type->kept == BRIDGE_TABLE || type->kept == BRIDGE_TOGGLE)
        set_take(r->in_table, type->id);
    if (type->kept == BRIDGE_SPARSE_SET)
        r->set_type[type->slot] = nullptr;
    if (type->kept == BRIDGE_DIRECT_INDEXED)
        r->direct_type[type->slot] = nullptr;
    if (type->cells_len)
        block_column_free(w, w->shared + type->cells, type->cells_len);
    type->set = false;
    r->types[type->id] = nullptr;
}
