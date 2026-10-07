/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** src/storage_drop.h -- the rows of a component nobody declares any more,
** dropped by the policy its last declarer stated: what the files that do it
** share.
**
** lifetime.md, *What unload leaves behind*
*/

#ifndef BRIDGE_SRC_STORAGE_DROP_H
    #define BRIDGE_SRC_STORAGE_DROP_H

    #include "storage_inside.h"

/*
** storage_drop_cells
**
** Drops a direct-indexed component: every entity that has been given one.
*/
void storage_drop_cells(storage_t *r, const storage_type_t *type,
    enum bridge_on_unload policy);

/*
** storage_drop_set
**
** Drops a component kept in a sparse set: an entity at a time, from the
** last.
*/
void storage_drop_set(storage_t *r, const storage_type_t *type,
    enum bridge_on_unload policy);

/*
** storage_drop_single
**
** Drops a singleton. It is on no entity, so the three policies say one
** thing: it has no value.
*/
void storage_drop_single(storage_t *r, storage_type_t *type);

/*
** storage_drop_table
**
** Drops a component from one table that carries it. Answers how many rows
** stay in a second table of their archetype, for want of room to join them
** to the first.
*/
uint32_t storage_drop_table(storage_t *r, uint32_t t,
    const storage_type_t *type, enum bridge_on_unload policy);

#endif
