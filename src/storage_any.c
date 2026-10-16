/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "storage_reach.h"
#include "storage_set.h"
#include "storage_sets.h"
#include "storage_view.h"

static bool has_any_cell(storage_t *r, const storage_type_t *type,
    const question_t *asked)
{
    const uint8_t *cells = r->b->shared + type->cells + COLUMN;
    const uint64_t *given = get_given(r, type->slot);
    uint32_t index = 1;

    while (index < r->high) {
        if (set_has(given, index)
            && asked->is(asked->ctx, cells + (size_t)index * type->size))
            return (true);
        index++;
    }
    return (false);
}

static bool has_any_row(storage_t *r, uint32_t t, const storage_type_t *type,
    const question_t *asked)
{
    const uint8_t *column = r->b->shared + get_columns(r, t)[type->id];
    uint32_t row = 0;

    while (row < r->tables[t].rows) {
        if (asked->is(asked->ctx, column + (size_t)row * type->size))
            return (true);
        row++;
    }
    return (false);
}

bool storage_has_any(storage_t *r, storage_type_t *type, storage_is_fn_t is,
    void *ctx)
{
    question_t asked = { is, ctx };
    uint32_t t = 1;

    if (type->kept == BRIDGE_SINGLETON)
        return (type->set && is(ctx, r->b->shared + type->cells));
    if (type->kept == BRIDGE_DIRECT_INDEXED && has_any_cell(r, type, &asked))
        return (true);
    while (t < r->table_high) {
        if (r->tables[t].live && set_has(get_set(r, t), type->id)
            && has_any_row(r, t, type, &asked))
            return (true);
        t++;
    }
    return (false);
}

static uint32_t count_off_rows(storage_t *r, uint32_t t,
    const storage_type_t *type)
{
    const table_t *tab = &r->tables[t];
    uint32_t column = get_columns(r, t)[type->id];
    uint32_t n = 0;
    uint32_t row = 0;

    while (row < tab->rows) {
        n += !is_on(r, column, tab->room, row);
        row++;
    }
    return (n);
}

uint32_t storage_count_off(storage_t *r, const storage_type_t *type)
{
    uint32_t n = 0;
    uint32_t t = 1;

    while (type->kept == BRIDGE_TOGGLE && t < r->table_high) {
        if (r->tables[t].live && set_has(get_set(r, t), type->id))
            n += count_off_rows(r, t, type);
        t++;
    }
    return (n);
}
