/*
** Copyright (C) 2026 Bridge Owner
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** src/storage_any.c -- the values of a type looked through: whether any
** answers a question, and how many toggled rows are off.
*/

#include "storage_reach.h"
#include "storage_set.h"
#include "storage_sets.h"
#include "storage_view.h"

/*
** has_any_cell
**
** Answers whether the question is true of any cell of a direct-indexed type
** that an entity has been given.
*/
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

/*
** has_any_row
**
** Answers whether the question is true of any row of a type in one table.
*/
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

/*
** storage_has_any
**
** Answers whether `is` answers true for any row of a type, handed the row's
** bytes.
*/
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

/*
** count_off_rows
**
** Answers how many rows of one table have a toggled type off.
*/
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

/*
** storage_count_off
**
** Answers how many of the entities that carry a toggled type have it off.
*/
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
