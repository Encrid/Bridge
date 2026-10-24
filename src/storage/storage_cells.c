/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <string.h>

#include "storage_publish.h"
#include "storage_reach.h"
#include "storage_view.h"

static void write_cells(publish_t *p, uint32_t t)
{
    const module_t *m = p->m;
    const uint32_t *columns = get_columns(p->r, t);
    uint32_t *row = p->cell + (size_t)t * m->declared_count;
    uint32_t id;
    uint32_t s = 0;

    p->rows[t] = p->r->tables[t].rows;
    p->blocks[t] = p->r->tables[t].block;
    while (s < m->declared_count) {
        id = m->declared[s]->type->id;
        row[s] = get_handed(p->r, id, columns[id]);
        s++;
    }
}

static uint32_t get_kept(const bridge_world_t *w, const storage_type_t *type)
{
    uint64_t index = 4ull * type->slot * w->entities;

    if (type->kept == BRIDGE_SPARSE_SET)
        return (w->sparse + (uint32_t)index);
    if (type->kept == BRIDGE_DIRECT_INDEXED)
        return (type->cells + (uint32_t)COLUMN);
    if (type->kept != BRIDGE_SINGLETON || !type->set)
        return (0);
    return (type->size ? type->cells : BRIDGE_CARRIED);
}

static void write_slots(publish_t *p)
{
    module_t *m = p->m;
    uint32_t s = 0;

    while (s < m->declared_count) {
        p->kept[s] = get_kept(p->r->w, m->declared[s]->type);
        p->stride[s] = m->declared[s]->type->size;
        p->write[s] = m->may_write[s] ? 1u : 0u;
        p->kind[s] = m->declared[s]->type->kept;
        s++;
    }
}

static void write_every_cell(publish_t *p)
{
    const storage_t *r = p->r;
    module_t *m = p->m;
    uint32_t tables = r->table_room + 1;
    uint32_t t = 1;

    (void)block_write_header(r->w, m);
    memset(p->cell, 0, 4u * m->declared_count * tables);
    memset(p->rows, 0, 4u * tables);
    memset(p->blocks, 0, 4u * tables);
    while (t < r->table_high) {
        if (r->tables[t].live)
            write_cells(p, t);
        t++;
    }
    write_slots(p);
}

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
