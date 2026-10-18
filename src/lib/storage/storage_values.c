/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "storage_reach.h"
#include "storage_set.h"
#include "storage_sets.h"
#include "storage_view.h"

static void read_cells(storage_t *r, const storage_type_t *type,
    const value_reader_t *reader)
{
    const uint8_t *cells = r->b->shared + type->cells + COLUMN;
    const uint64_t *given = get_given(r, type->slot);

    for (uint32_t index = 1; index < r->high; index++)
        if (set_has(given, index))
            reader->read(reader->ctx, cells + (size_t)index * type->size);
}

static void read_rows(storage_t *r, uint32_t t, const storage_type_t *type,
    const value_reader_t *reader)
{
    const uint8_t *column = r->b->shared + get_columns(r, t)[type->id];

    for (uint32_t row = 0; row < r->tables[t].rows; row++)
        reader->read(reader->ctx, column + (size_t)row * type->size);
}

void storage_visit_values(storage_t *r, storage_type_t *type,
    storage_read_fn_t read, void *ctx)
{
    value_reader_t reader = { read, ctx };

    if (type->kept == BRIDGE_SINGLETON) {
        if (type->set)
            read(ctx, r->b->shared + type->cells);
        return;
    }
    if (type->kept == BRIDGE_DIRECT_INDEXED)
        read_cells(r, type, &reader);
    for (uint32_t t = 1; t < r->table_high; t++)
        if (r->tables[t].live && set_has(get_set(r, t), type->id))
            read_rows(r, t, type, &reader);
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
