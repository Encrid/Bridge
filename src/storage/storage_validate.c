/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "hash.h"
#include "storage_reach.h"
#include "storage_sets.h"
#include "storage_walk.h"

static uint64_t *get_col_hash(const storage_t *r, uint32_t t)
{
    return (r->col_hash + (size_t)t * r->type_room);
}

static uint64_t hash_column(const storage_t *r, uint32_t t, uint32_t id)
{
    const table_t *tab = &r->tables[t];
    size_t len = (size_t)r->types[id]->size * tab->rows;

    return (hash_fnv1a(r->w->shared + get_columns(r, t)[id], len));
}

static void scan_table(storage_t *r, uint32_t t, storage_changed_fn_t fn,
    void *ctx)
{
    uint64_t *hashes = get_col_hash(r, t);
    walk_t walk = walk_set(get_set(r, t));
    uint32_t id = walk_next(r, &walk);
    uint64_t h;

    while (id != r->type_room) {
        h = hash_column(r, t, id);
        if (h != hashes[id] && fn)
            fn(ctx, r->types[id]);
        hashes[id] = h;
        id = walk_next(r, &walk);
    }
}

void storage_validate_scan(storage_t *r, storage_changed_fn_t fn, void *ctx)
{
    uint32_t t = 1;

    while (t < r->table_high) {
        if (r->tables[t].live)
            scan_table(r, t, fn, ctx);
        t++;
    }
}

void storage_validate_baseline(storage_t *r)
{
    storage_validate_scan(r, nullptr, nullptr);
}
