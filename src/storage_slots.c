/*
** Copyright (C) 2026 Bridge Owner
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** src/storage_slots.c -- what is left of what a budget counts, what a module
** may write, and a type forgotten.
*/

#include <stdlib.h>

#include "storage_set.h"

/*
** storage_count_free_sets
**
** Answers how many more types may be kept in a sparse set.
*/
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

/*
** storage_count_free_direct
**
** Answers how many more types may be kept direct-indexed.
*/
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

/*
** storage_make_writable
**
** Makes the set of the types a module may write. false if there was not the
** memory for it.
*/
bool storage_make_writable(storage_t *r, module_t *m)
{
    if (!m->writable)
        m->writable = calloc(r->words, sizeof(*m->writable));
    return (m->writable != nullptr);
}

/*
** storage_let_write
**
** Records that a module may write a type.
*/
void storage_let_write([[maybe_unused]] storage_t *r, module_t *m,
    storage_type_t *type)
{
    set_put(m->writable, type->id);
}

/*
** storage_undeclare
**
** Forgets a type nobody declares any more, and gives back the block a
** direct-indexed type or a singleton had.
*/
void storage_undeclare(storage_t *r, storage_type_t *type)
{
    bridge_t *b = r->b;

    r->apart -= type->kept != BRIDGE_TABLE && type->kept != BRIDGE_TOGGLE;
    r->tags -= !type->size;
    if (type->kept == BRIDGE_TABLE || type->kept == BRIDGE_TOGGLE)
        set_take(r->in_table, type->id);
    if (type->kept == BRIDGE_SPARSE_SET)
        r->set_type[type->slot] = nullptr;
    if (type->kept == BRIDGE_DIRECT_INDEXED)
        r->direct_type[type->slot] = nullptr;
    if (type->cells_len)
        block_column_free(b, b->shared + type->cells, type->cells_len);
    type->set = false;
    r->types[type->id] = nullptr;
}
