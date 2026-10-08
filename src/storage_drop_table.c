/*
** Copyright (C) 2026 Bridge Owner
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** src/storage_drop_table.c -- a component dropped from one table that
** carries it: its entities destroyed, moved to the table of what is left,
** or the table stripped of the column where it stands.
*/

#include "storage_drop.h"
#include "storage_grow.h"
#include "storage_strip.h"
#include "storage_unload.h"

/*
** sweep_table
**
** Empties a table whose entities the policy destroys, from the last row, so
** that none is moved into the place of one already seen. Under `sweep`, an
** entity that carries something not kept in a table carries something
** still: it is not destroyed, and leaves the table for none.
*/
static void sweep_table(storage_t *r, uint32_t t, const storage_type_t *type,
    enum bridge_on_unload policy)
{
    table_t *tab = &r->tables[t];
    uint64_t id;

    while (tab->live) {
        id = get_who(r, tab)[tab->rows - 1];
        if (policy == BRIDGE_DELETE || !is_apart(r, (uint32_t)id)) {
            destroy_entity(r, (uint32_t)id);
        } else {
            see_less(r, (uint32_t)id, type, r->masked);
            tell_observers(r, &(seen_t){ id, r->before, r->masked, nullptr });
            move_row(r, (uint32_t)id, 0);
        }
        if (tab->live && !tab->rows)
            free_table(r, t);
    }
}

/*
** tell_rows
**
** Tells whoever observes them that each entity of a table loses a type,
** from the last row to the first.
*/
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

/*
** move_rows
**
** Moves every row of a table to table `to`, which has room for them all, or
** to none, and gives the emptied table back.
*/
static void move_rows(storage_t *r, uint32_t t, uint32_t to)
{
    const table_t *tab = &r->tables[t];

    while (tab->rows)
        move_row(r, (uint32_t)get_who(r, tab)[tab->rows - 1], to);
    free_table(r, t);
}

/*
** keep_rows
**
** Keeps a table's rows without a type. With nothing left to carry, or a
** table of what is left with room for them all, they move there. With no
** table of what is left, this one becomes it; and with one that has no
** room, the column goes where the table stands, and until either empties
** there are two. Answers how many rows stay in such a second table.
*/
static uint32_t keep_rows(storage_t *r, uint32_t t, const storage_type_t *type,
    const uint64_t *left)
{
    const table_t *tab = &r->tables[t];
    bool none = set_is_empty(r, left);
    uint32_t to = none ? 0 : find_table(r, left);
    uint32_t rows = tab->rows;

    if (none || (to && grow_table(r, to, r->tables[to].rows + rows))) {
        move_rows(r, t, to);
        return (0);
    }
    strip_table(r, t, type, !to);
    return (to ? rows : 0);
}

/*
** storage_drop_table
**
** Drops a component from one table that carries it, by the policy stated.
*/
uint32_t storage_drop_table(storage_t *r, uint32_t t,
    const storage_type_t *type, enum bridge_on_unload policy)
{
    uint64_t *left = r->scratch;

    set_copy(r, left, get_set(r, t));
    set_take(left, type->id);
    if (is_swept(r, policy, left)) {
        sweep_table(r, t, type, policy);
        return (0);
    }
    tell_rows(r, t, type);
    return (keep_rows(r, t, type, left));
}
