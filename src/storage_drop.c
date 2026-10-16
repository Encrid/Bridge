/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "storage_drop.h"
#include "storage_set.h"
#include "storage_sets.h"

static void say_apart(storage_t *r, const storage_type_t *type, uint32_t rows)
{
    const bridge_t *b = r->b;
    say_t about = { nullptr, nullptr, BRIDGE_WARN };
    uint32_t i = 0;

    while (b->types[i]->type != type)
        i++;
    say_about(r->b, &about, "'%s' is stripped, and %u rows stay in a second "
        "table of their archetype: [budget] columns of %u bytes has no room "
        "to join them to the first", b->types[i]->name, rows, b->columns_len);
}

static void drop_tables(storage_t *r, const storage_type_t *type,
    enum bridge_on_unload policy)
{
    uint32_t apart = 0;
    uint32_t t = 1;

    while (t < r->table_high) {
        if (r->tables[t].live && set_has(get_set(r, t), type->id))
            apart += storage_drop_table(r, t, type, policy);
        t++;
    }
    if (apart)
        say_apart(r, type, apart);
}

void storage_drop(storage_t *r, storage_type_t *type,
    enum bridge_on_unload policy)
{
    if (type->kept == BRIDGE_SPARSE_SET)
        storage_drop_set(r, type, policy);
    else if (type->kept == BRIDGE_DIRECT_INDEXED)
        storage_drop_cells(r, type, policy);
    else if (type->kept == BRIDGE_SINGLETON)
        storage_drop_single(r, type);
    if (type->kept == BRIDGE_TABLE || type->kept == BRIDGE_TOGGLE)
        drop_tables(r, type, policy);
}
