/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "storage_drop.h"
#include "storage_set.h"
#include "storage_sets.h"

static const char *get_name(const storage_t *r, const storage_type_t *type)
{
    const bridge_world_t *w = r->w;
    uint32_t i = 0;

    while (w->types[i]->type != type)
        i++;
    return (w->types[i]->name);
}

static void say_apart(storage_t *r, const storage_type_t *type, uint32_t rows)
{
    say_t about = { nullptr, nullptr, BRIDGE_WARN };

    say_about(r->w, &about, "'%s' is stripped, and %u rows stay in a second "
        "table of their archetype: [budget] columns of %u bytes has no room "
        "to join them to the first", get_name(r, type), rows,
        r->w->columns_len);
}

static void say_deleted(storage_t *r, const storage_type_t *type,
    uint32_t count)
{
    say_t about = { nullptr, nullptr, BRIDGE_WARN };

    say_about(r->w, &about, "'%s' is deleted, and %u entities lose only it: "
        "[budget] tables of %u or [budget] columns of %u bytes has no room "
        "for their husks", get_name(r, type), count, r->w->tables,
        r->w->columns_len);
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
    uint32_t left;

    if (type->kept == BRIDGE_SINGLETON) {
        storage_drop_single(r, type);
        return;
    }
    if (policy == BRIDGE_DELETE) {
        left = storage_delete_carriers(r, type);
        if (left)
            say_deleted(r, type, left);
        policy = BRIDGE_STRIP;
    }
    if (type->kept == BRIDGE_SPARSE_SET)
        storage_drop_set(r, type, policy);
    else if (type->kept == BRIDGE_DIRECT_INDEXED)
        storage_drop_cells(r, type, policy);
    else
        drop_tables(r, type, policy);
}
