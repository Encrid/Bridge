/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "storage_apply.h"
#include "storage_change.h"
#include "storage_drop.h"
#include "storage_forget.h"
#include "storage_room.h"

void storage_let_keep(storage_t *r, const storage_type_t *type, bool keep)
{
    set_write(r->husk_types, type->id, keep);
}

static void fold_carrier(storage_t *r, uint32_t index, void *ctx)
{
    const storage_type_t *type = ctx;

    begin_entity(r, index);
    set_take(get_work(r, index), type->id);
    fold_husk(r, index);
}

uint32_t storage_delete_carriers(storage_t *r, storage_type_t *type)
{
    storage_visit_carriers(r, type, fold_carrier, type);
    reserve_all(r);
    tell_all(r);
    place_all(r);
    forget_all(r);
    return (storage_count_carriers(r, type));
}
