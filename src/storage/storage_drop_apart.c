/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "storage_drop.h"
#include "storage_unload.h"

static void drop_cell(storage_t *r, const storage_type_t *type,
    enum bridge_on_unload policy, uint32_t index)
{
    uint64_t id = (uint64_t)r->places[index].generation << 32 | index;

    see_less(r, index, type, r->scratch);
    if (is_swept(r, index, policy, r->scratch)) {
        destroy_entity(r, index);
        return;
    }
    tell_observers(r, &(seen_t){ id, r->before, r->scratch, nullptr });
    give_cell(r, type->slot, index, false);
}

void storage_drop_cells(storage_t *r, const storage_type_t *type,
    enum bridge_on_unload policy)
{
    uint32_t index = 1;

    while (index < r->high) {
        if (set_has(get_given(r, type->slot), index))
            drop_cell(r, type, policy, index);
        index++;
    }
    sweep_sets(r);
}

static void drop_member(storage_t *r, const storage_type_t *type,
    enum bridge_on_unload policy, uint64_t id)
{
    uint32_t index = (uint32_t)id;

    see_less(r, index, type, r->scratch);
    if (is_swept(r, index, policy, r->scratch)) {
        destroy_entity(r, index);
        return;
    }
    tell_observers(r, &(seen_t){ id, r->before, r->scratch, nullptr });
    leave_set(r, type->slot, index);
}

void storage_drop_set(storage_t *r, const storage_type_t *type,
    enum bridge_on_unload policy)
{
    uint32_t t = r->set_table[type->slot];
    const table_t *tab = &r->tables[t];

    while (t && tab->rows) {
        drop_member(r, type, policy, get_who(r, tab)[tab->rows - 1]);
        t = r->set_table[type->slot];
        tab = &r->tables[t];
    }
    sweep_sets(r);
}

void storage_drop_single(storage_t *r, storage_type_t *type)
{
    if (!type->set)
        return;
    see_less(r, 0, type, r->scratch);
    tell_observers(r, &(seen_t){ 0, r->before, r->scratch, nullptr });
    set_single(r, type, false);
}
