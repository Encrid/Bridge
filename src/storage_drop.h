/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#ifndef BRIDGE_SRC_STORAGE_DROP_H
    #define BRIDGE_SRC_STORAGE_DROP_H

    #include "storage_inside.h"

void storage_drop_cells(storage_t *r, const storage_type_t *type,
    enum bridge_on_unload policy);

void storage_drop_set(storage_t *r, const storage_type_t *type,
    enum bridge_on_unload policy);

void storage_drop_single(storage_t *r, storage_type_t *type);

uint32_t storage_drop_table(storage_t *r, uint32_t t,
    const storage_type_t *type, enum bridge_on_unload policy);

#endif
