/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdlib.h>

#include "bits.h"
#include "storage_test.h"

uint32_t storage_test_random(storage_test_both_t *b, uint32_t below)
{
    b->seed ^= b->seed << 13;
    b->seed ^= b->seed >> 7;
    b->seed ^= b->seed << 17;
    return ((uint32_t)(b->seed % below));
}

static void fill_system(storage_test_both_t *b, bridge_system_t *s)
{
    uint32_t pick;

    for (uint32_t i = 0; i < s->terms; i++) {
        pick = storage_test_random(b, BOTH_TYPES + 1);
        s->term[i].touches = pick < BOTH_TYPES ? &b->types[pick] : nullptr;
        s->term[i].writes = storage_test_random(b, 4) == 0;
    }
    if (storage_test_random(b, 4) == 0) {
        s->changes[0] = &b->types[storage_test_random(b, BOTH_TYPES)];
        s->change_count = 1;
    }
    s->phase = storage_test_random(b, 3);
}

static void list_one(storage_test_both_t *b, uint32_t record, uint32_t j)
{
    bridge_world_t *w = &b->w;
    module_t *m = &b->mods[record];
    bridge_system_t *s = storage_test_make_system(m,
        1 + storage_test_random(b, 2), 1, bits_words(j) + 1);

    if (!s) {
        b->differ++;
        return;
    }
    s->slot = j;
    s->full = b->fulls[record % BOTH_MODULES][j];
    s->named = schedule_find_name(&w->order, s->full);
    fill_system(b, s);
    for (uint32_t i = 0; i < w->system_count; i++)
        if (w->systems[i]->m == m && storage_test_random(b, 2))
            schedule_join_before(s, w->systems[i]);
    w->systems[w->system_count++] = s;
    s->fresh = true;
    s->dirty = true;
    schedule_index_system(s);
}

void storage_test_add_module(storage_test_both_t *b, uint32_t record)
{
    uint32_t count = 1 + storage_test_random(b, BOTH_SYSTEMS);

    for (uint32_t j = 0; j < count; j++)
        list_one(b, record, j);
}

void storage_test_drop_module(storage_test_both_t *b, uint32_t record)
{
    bridge_world_t *w = &b->w;
    const module_t *m = &b->mods[record];
    uint32_t kept = 0;

    schedule_leave_module(w, m);
    for (uint32_t i = 0; i < w->system_count; i++) {
        if (w->systems[i]->m == m)
            free(w->systems[i]);
        else
            w->systems[kept++] = w->systems[i];
    }
    w->system_count = kept;
}
