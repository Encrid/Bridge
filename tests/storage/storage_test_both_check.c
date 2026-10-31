/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>
#include <string.h>

#include "storage_test.h"

void storage_test_name_both(storage_test_both_t *b)
{
    for (uint32_t k = 0; k < BOTH_MODULES * BOTH_SYSTEMS; k++)
        snprintf(b->fulls[k / BOTH_SYSTEMS][k % BOTH_SYSTEMS],
            sizeof(b->fulls[0][0]), "game:m%u:s%u", k / BOTH_SYSTEMS,
            k % BOTH_SYSTEMS);
    for (uint32_t k = 0; k < BOTH_TYPES; k++)
        snprintf(b->kinds_of[k], sizeof(b->kinds_of[k]), "game:T%u", k);
}

void storage_test_pair_names(storage_test_both_t *b, bridge_order_t *pairs,
    uint32_t count)
{
    const uint32_t names = BOTH_MODULES * BOTH_SYSTEMS;
    uint32_t turn = 5 + 2 * storage_test_random(b, 2) + 6 * (b->seed & 1);
    uint32_t x;
    uint32_t y;
    uint32_t swap;

    for (uint32_t i = 0; i < count; i++) {
        x = storage_test_random(b, names);
        y = (x + 1 + storage_test_random(b, names - 1)) % names;
        if (storage_test_random(b, 2))
            y = x - x % BOTH_SYSTEMS + (x % BOTH_SYSTEMS + 1
                + storage_test_random(b, BOTH_SYSTEMS - 1)) % BOTH_SYSTEMS;
        if ((x * turn) % names > (y * turn) % names) {
            swap = x;
            x = y;
            y = swap;
        }
        pairs[i].before = b->fulls[x / BOTH_SYSTEMS][x % BOTH_SYSTEMS];
        pairs[i].after = b->fulls[y / BOTH_SYSTEMS][y % BOTH_SYSTEMS];
    }
}

static void check_both(storage_test_both_t *b, int refused)
{
    bridge_world_t *w = &b->w;
    char why[256] = "";
    int again;

    for (uint32_t i = 0; i < w->system_count; i++) {
        b->levels[i] = w->systems[i]->level;
        b->marks[i][0] = w->systems[i]->fresh;
        b->marks[i][1] = w->systems[i]->dirty;
    }
    again = storage_test_build_all(w, why, sizeof(why));
    b->differ += again != refused || strcmp(why, b->why);
    for (uint32_t i = 0; i < w->system_count; i++) {
        b->differ += w->systems[i]->level != b->levels[i];
        w->systems[i]->fresh = b->marks[i][0];
        w->systems[i]->dirty = b->marks[i][1];
    }
}

int storage_test_build_once(storage_test_both_t *b)
{
    int refused;

    b->why[0] = '\0';
    refused = schedule_build(&b->w, b->why, sizeof(b->why));
    b->builds++;
    if (refused && strstr(b->why, "different phases"))
        b->kinds[0]++;
    else if (refused && strstr(b->why, "before itself"))
        b->kinds[1]++;
    else if (refused)
        b->kinds[2]++;
    check_both(b, refused);
    return (refused);
}

void storage_test_set_parked(storage_test_both_t *b, uint32_t record,
    bool parked)
{
    bridge_world_t *w = &b->w;
    bridge_system_t *s;

    for (uint32_t i = 0; i < w->system_count; i++) {
        s = w->systems[i];
        if (s->m == &b->mods[record] && parked)
            abi_park(w, s);
        else if (s->m == &b->mods[record]) {
            s->parked = 0;
            s->dirty = true;
            w->parked--;
        }
    }
}
