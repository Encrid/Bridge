/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <string.h>

#include "storage_unload.h"

static uint32_t take_index(storage_t *r)
{
    uint32_t index = r->free_head;

    if (index) {
        r->free_head = r->places[index].row;
        return (index);
    }
    if (r->high >= r->w->entities)
        return (0);
    index = r->high;
    r->high++;
    r->places[index].generation = 1;
    return (index);
}

uint32_t storage_reserve(storage_t *r, uint64_t *ids, uint32_t want)
{
    uint32_t count = 0;
    uint32_t index = count < want ? take_index(r) : 0;
    place_t *p;

    while (index) {
        p = &r->places[index];
        p->table = 0;
        p->row = HELD;
        ids[count] = (uint64_t)p->generation << 32 | index;
        count++;
        index = count < want ? take_index(r) : 0;
    }
    return (count);
}

void storage_release(storage_t *r, const uint64_t *ids, uint32_t count)
{
    uint32_t i = 0;

    while (i < count) {
        destroy_entity(r, (uint32_t)ids[i]);
        i++;
    }
    sweep_sets(r);
}

bool storage_is_alive(storage_t *r, uint64_t id)
{
    return (is_alive(r, id));
}

void storage_give_single(storage_t *r, storage_type_t *type,
    const void *bytes, uint32_t size)
{
    uint64_t *set = r->scratch;
    uint64_t *attached = r->masked;

    get_carried(r, 0, r->before);
    set_copy(r, set, r->before);
    set_put(set, type->id);
    set_clear(r, attached);
    set_put(attached, type->id);
    tell_observers(r, &(seen_t){ 0, r->before, set, attached });
    set_single(r, type, true);
    memcpy(r->w->shared + type->cells, bytes, size);
}
