/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <string.h>

#include "storage_bytes.h"
#include "storage_reach.h"
#include "storage_set.h"

static uint32_t find_slot(const storage_t *r, enum bridge_storage kept)
{
    uint32_t slot = 0;

    while (kept == BRIDGE_SPARSE_SET && r->set_type[slot])
        slot++;
    while (kept == BRIDGE_DIRECT_INDEXED && r->direct_type[slot])
        slot++;
    return (slot);
}

static uint8_t *take_zeroed(storage_t *r, uint64_t want)
{
    uint8_t *block = nullptr;

    if (want <= INT32_MAX)
        block = block_column_regrow(r->b, nullptr, 0, (uint32_t)want);
    if (block)
        memset(block, 0, (size_t)want);
    return (block);
}

static bool take_cells(storage_t *r, storage_type_t *type)
{
    bool direct = type->kept == BRIDGE_DIRECT_INDEXED;
    uint64_t cells = COLUMN + (uint64_t)type->size * r->b->entities;
    uint64_t want = pad_column(direct ? cells : type->size + !type->size);
    uint32_t bits = r->b->direct + (uint32_t)(8ull * type->slot * r->bit_words);
    uint8_t *block = take_zeroed(r, want);

    if (!block)
        return (false);
    type->cells = (uint32_t)(block - r->b->shared);
    type->cells_len = (uint32_t)want;
    if (!direct)
        return (true);
    memcpy(block, &bits, 4);
    memset(get_given(r, type->slot), 0, 8u * r->bit_words);
    memset(get_shown(r, type->slot), 0, 8u * r->bit_words);
    return (true);
}

static void file_type(storage_t *r, storage_type_t *type)
{
    enum bridge_storage kept = type->kept;

    r->types[type->id] = type;
    if (kept == BRIDGE_SPARSE_SET)
        r->set_type[type->slot] = type;
    if (kept == BRIDGE_DIRECT_INDEXED)
        r->direct_type[type->slot] = type;
    if (kept == BRIDGE_TABLE || kept == BRIDGE_TOGGLE)
        set_put(r->in_table, type->id);
    r->apart += kept != BRIDGE_TABLE && kept != BRIDGE_TOGGLE;
    r->tags += !type->size;
}

storage_type_t *storage_declare(storage_t *r, uint32_t size, uint32_t align,
    enum bridge_storage kept)
{
    storage_type_t type = { .size = size, .align = align, .kept = kept };
    bool blocked = kept == BRIDGE_DIRECT_INDEXED || kept == BRIDGE_SINGLETON;

    while (r->types[type.id])
        type.id++;
    type.slot = find_slot(r, kept);
    if (blocked && !take_cells(r, &type))
        return (nullptr);
    r->type_pool[type.id] = type;
    file_type(r, &r->type_pool[type.id]);
    return (&r->type_pool[type.id]);
}
