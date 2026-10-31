/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "storage_test.h"

static char FIRST[] = "first";
static char SECOND[] = "second";
static char *PHASES[2] = { FIRST, SECOND };

static bool open_world(storage_test_both_t *b, storage_test_t *t)
{
    bridge_world_t *w = &b->w;
    bridge_order_t pairs[4];
    uint32_t count = storage_test_random(b, 5);

    *w = (bridge_world_t){ .phases = PHASES, .phase_count = 2,
        .systems = b->list, .sink = storage_test_hear, .ud = t };
    memset(b->types, 0, sizeof(b->types));
    for (uint32_t k = 0; k < BOTH_TYPES; k++)
        b->types[k].name = b->kinds_of[k];
    storage_test_pair_names(b, pairs, count);
    w->build_in = calloc(BOTH_ROOM, sizeof(*w->build_in));
    w->build_bits = calloc(schedule_get_matrix_words(BOTH_ROOM),
        sizeof(*w->build_bits));
    w->run_jobs = calloc(BOTH_ROOM, sizeof(schedule_job_t));
    w->run_lists = calloc(BOTH_ROOM, sizeof(*w->run_lists));
    return (w->build_in && w->build_bits && w->run_jobs && w->run_lists
        && !schedule_order(w, pairs, count));
}

static void reload(storage_test_both_t *b, uint32_t k)
{
    uint32_t old = k + BOTH_MODULES * b->current[k];
    uint32_t next = k + BOTH_MODULES * !b->current[k];

    storage_test_set_parked(b, old, true);
    storage_test_add_module(b, next);
    if (storage_test_build_once(b)) {
        storage_test_drop_module(b, next);
        storage_test_set_parked(b, old, false);
    } else {
        storage_test_drop_module(b, old);
        b->current[k] = !b->current[k];
    }
    (void)storage_test_build_once(b);
}

static void step(storage_test_both_t *b)
{
    uint32_t k = storage_test_random(b, BOTH_MODULES);
    uint32_t record = k + BOTH_MODULES * b->current[k];

    if (!b->loaded[k]) {
        storage_test_add_module(b, record);
        b->loaded[k] = !storage_test_build_once(b);
        if (b->loaded[k])
            return;
    } else if (storage_test_random(b, 2)) {
        b->loaded[k] = false;
    } else {
        reload(b, k);
        return;
    }
    storage_test_drop_module(b, record);
    (void)storage_test_build_once(b);
}

static void close_world(storage_test_both_t *b)
{
    bridge_world_t *w = &b->w;

    for (uint32_t k = 0; k < 2 * BOTH_MODULES; k++)
        storage_test_drop_module(b, k);
    memset(b->current, 0, sizeof(b->current));
    memset(b->loaded, 0, sizeof(b->loaded));
    schedule_free_pairs(&w->order);
    free(w->build_in);
    free(w->build_bits);
    free(w->run_jobs);
    free(w->run_lists);
}

enum storage_test_end storage_test_build_both(storage_test_t *t)
{
    storage_test_both_t *b = calloc(1, sizeof(*b));
    uint32_t worlds = t->more ? (uint32_t)strtoul(t->more, nullptr, 10) : 0;

    if (!b)
        return (STORAGE_TEST_FAILED);
    b->seed = 0x9e3779b97f4a7c15u;
    storage_test_name_both(b);
    for (uint32_t i = 0; i < worlds; i++) {
        b->differ += !open_world(b, t);
        for (uint32_t s = 0; s < 16; s++)
            step(b);
        close_world(b);
    }
    printf("both ways: %u worlds, %u builds, refused %u for 2 phases, %u for "
        "a cycle and %u for a conflict, %u differ\n", worlds, b->builds,
        b->kinds[0], b->kinds[1], b->kinds[2], b->differ);
    free(b);
    return (STORAGE_TEST_DONE);
}
