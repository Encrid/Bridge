/*
** Copyright (C) 2026 Bridge Owner
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** src/storage_cells.c -- a module's column_base: for each table and each
** type it declared, where that type's column is.
*/

#include <string.h>

#include "storage_publish.h"
#include "storage_reach.h"
#include "storage_view.h"

/*
** write_cells
**
** Writes the row of a module's column_base that is table `t`.
*/
static void write_cells(publish_t *p, uint32_t t)
{
    const module_t *m = p->m;
    const uint32_t *columns = get_columns(p->r, t);
    uint32_t *row = p->cell + (size_t)t * m->declared_count;
    uint32_t id;
    uint32_t s = 0;

    while (s < m->declared_count) {
        id = m->declared[s]->type->id;
        row[s] = get_handed(p->r, id, columns[id]);
        s++;
    }
}

/*
** get_kept
**
** Answers where a module reaches a component that is not kept in a table: a
** sparse set's index, a direct-indexed component's cells, a singleton's
** value while it has one. 0 for a component kept in a table, and for a
** singleton with no value.
*/
static uint32_t get_kept(const bridge_t *b, const storage_type_t *type)
{
    uint64_t index = 4ull * type->slot * b->entities;

    if (type->kept == BRIDGE_SPARSE_SET)
        return (b->sparse + (uint32_t)index);
    if (type->kept == BRIDGE_DIRECT_INDEXED)
        return (type->cells + (uint32_t)COLUMN);
    if (type->kept != BRIDGE_SINGLETON || !type->set)
        return (0);
    return (type->size ? type->cells : BRIDGE_CARRIED);
}

/*
** write_every_cell
**
** Writes a module's whole column_base, with its header: every live table,
** and where each component kept apart is reached.
*/
static void write_every_cell(publish_t *p)
{
    const storage_t *r = p->r;
    module_t *m = p->m;
    uint32_t tables = r->table_room + 1;
    uint32_t t = 1;
    uint32_t s = 0;

    block_write_header(r->b, m);
    memset(p->cell, 0, 4u * m->declared_count * tables);
    while (t < r->table_high) {
        if (r->tables[t].live)
            write_cells(p, t);
        t++;
    }
    while (s < m->declared_count) {
        p->kept[s] = get_kept(r->b, m->declared[s]->type);
        s++;
    }
}

/*
** storage_publish_cells
**
** Writes a module's column_base: every table when a declaration changed,
** and otherwise the ones touched.
*/
void storage_publish_cells(publish_t *p)
{
    const storage_t *r = p->r;
    uint32_t i = 0;

    if (p->all) {
        write_every_cell(p);
        return;
    }
    while (i < r->dirty_count) {
        write_cells(p, r->dirty[i]);
        i++;
    }
}
