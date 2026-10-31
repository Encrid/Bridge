/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "storage_inside.h"

static uint32_t free_entities(const storage_t *r)
{
    uint32_t count = 0;
    uint32_t index = r->free_head;

    while (index) {
        count++;
        index = r->places[index].row;
    }
    return (count);
}

storage_usage_t storage_usage(const storage_t *r)
{
    storage_usage_t u = {};

    u.entities_peak = r->high - 1;
    u.entities = u.entities_peak - free_entities(r);
    u.tables_peak = r->table_high - 1;
    u.tables = r->table_count;
    return (u);
}

bool storage_is_reshaped(const storage_t *r)
{
    uint32_t i = 0;

    while (i < r->dirty_count && !r->tables[r->dirty[i]].reshaped)
        i++;
    return (i < r->dirty_count);
}
