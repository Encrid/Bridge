/*
** Copyright (C) 2026 Bridge Owner
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** src/storage_carriers.c -- every entity that carries a type, visited,
** whichever way the type is kept.
*/

#include "storage_reach.h"
#include "storage_set.h"
#include "storage_sets.h"
#include "storage_view.h"

/*
** visit_given
**
** Visits every entity that has been given a direct-indexed component.
*/
static void visit_given(storage_t *r, const storage_type_t *type,
    storage_visit_fn_t visit, void *ctx)
{
    const uint64_t *given = get_given(r, type->slot);
    uint32_t index = 1;

    while (index < r->high) {
        if (set_has(given, index))
            visit(r, index, ctx);
        index++;
    }
}

/*
** visit_rows
**
** Visits the entity of every row of one table.
*/
static void visit_rows(storage_t *r, uint32_t t, storage_visit_fn_t visit,
    void *ctx)
{
    const table_t *tab = &r->tables[t];
    uint32_t row = 0;

    while (row < tab->rows) {
        visit(r, (uint32_t)get_who(r, tab)[row], ctx);
        row++;
    }
}

/*
** storage_visit_carriers
**
** Calls `visit` for every entity that carries a type. A singleton is on
** index 0, which is no entity, when it has a value; a type kept in a sparse
** set is in that set's table, as a type kept in tables is in its own.
*/
void storage_visit_carriers(storage_t *r, const storage_type_t *type,
    storage_visit_fn_t visit, void *ctx)
{
    uint32_t t = 1;

    if (type->kept == BRIDGE_SINGLETON && type->set)
        visit(r, 0, ctx);
    if (type->kept == BRIDGE_DIRECT_INDEXED)
        visit_given(r, type, visit, ctx);
    if (type->kept == BRIDGE_SINGLETON || type->kept == BRIDGE_DIRECT_INDEXED)
        return;
    while (t < r->table_high) {
        if (r->tables[t].live && set_has(get_set(r, t), type->id))
            visit_rows(r, t, visit, ctx);
        t++;
    }
}

static void count_one([[maybe_unused]] storage_t *r,
    [[maybe_unused]] uint32_t index, void *ctx)
{
    uint32_t *n = ctx;

    (*n)++;
}

/*
** storage_count_carriers
**
** Answers how many entities carry a type.
*/
uint32_t storage_count_carriers(storage_t *r, const storage_type_t *type)
{
    uint32_t n = 0;

    storage_visit_carriers(r, type, count_one, &n);
    return (n);
}
