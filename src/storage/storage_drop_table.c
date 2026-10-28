/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "storage_drop.h"
#include "storage_grow.h"
#include "storage_strip.h"
#include "storage_unload.h"

static void sweep_table(storage_t *r, uint32_t t, const storage_type_t *type,
    enum bridge_on_unload policy)
{
    table_t *tab = &r->tables[t];
    uint64_t id;

    while (tab->live) {
        id = get_who(r, tab)[tab->rows - 1];
        see_less(r, (uint32_t)id, type, r->masked);
        if (is_swept(r, (uint32_t)id, policy, r->masked)) {
            destroy_entity(r, (uint32_t)id);
        } else {
            tell_observers(r, &(seen_t){ id, r->before, r->masked, nullptr });
            move_row(r, (uint32_t)id, 0);
        }
        if (tab->live && !tab->rows)
            free_table(r, t);
    }
}

static void tell_rows(storage_t *r, uint32_t t, const storage_type_t *type)
{
    const table_t *tab = &r->tables[t];
    const uint64_t *who = get_who(r, tab);
    uint32_t row = tab->rows;

    while (row) {
        row--;
        see_less(r, (uint32_t)who[row], type, r->masked);
        tell_observers(r, &(seen_t){ who[row], r->before, r->masked, nullptr });
    }
}

static void move_rows(storage_t *r, uint32_t t, uint32_t to)
{
    const table_t *tab = &r->tables[t];

    while (tab->rows)
        move_row(r, (uint32_t)get_who(r, tab)[tab->rows - 1], to);
    free_table(r, t);
}

static uint32_t keep_rows(storage_t *r, uint32_t t, const storage_type_t *type,
    const uint64_t *left)
{
    uint32_t to = find_table(r, left);
    uint32_t rows = r->tables[t].rows;

    if (to && grow_table(r, to, r->tables[to].rows + rows)) {
        move_rows(r, t, to);
        return (0);
    }
    strip_table(r, t, type, !to);
    return (to ? rows : 0);
}

uint32_t storage_drop_table(storage_t *r, uint32_t t,
    const storage_type_t *type, enum bridge_on_unload policy)
{
    uint64_t *left = r->scratch;

    set_copy(r, left, get_set(r, t));
    set_take(left, type->id);
    if (set_is_empty(r, left)) {
        sweep_table(r, t, type, policy);
        return (0);
    }
    tell_rows(r, t, type);
    return (keep_rows(r, t, type, left));
}
