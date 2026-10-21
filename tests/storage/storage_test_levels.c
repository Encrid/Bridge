/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>
#include <stdlib.h>

#include "bits.h"
#include "storage_test.h"

constexpr uint32_t CHAIN_SYSTEMS = 100;

static uint64_t reach_tests;

static bool count_reach(const schedule_matrix_t *m, uint32_t i, uint32_t j)
{
    reach_tests++;
    return (schedule_reaches(m, i, j));
}

int counted_build(bridge_world_t *w, char *why, size_t why_len);

#define schedule_reaches count_reach
#define schedule_build counted_build
#include "schedule_build.c"
#undef schedule_build
#undef schedule_reaches

static bridge_system_t *make_system(module_t *m, declared_type_t *c,
    uint32_t slot)
{
    bridge_system_t *s = calloc(1, sizeof(*s) + sizeof(system_term_t)
        + bits_words(slot) * sizeof(uint64_t));

    if (!s)
        return (nullptr);
    s->m = m;
    s->terms = 1;
    s->term[0] = (system_term_t){ c, true };
    s->slot = slot;
    s->stated_before = (uint64_t *)(void *)&s->term[1];
    return (s);
}

static bool fill_world(bridge_world_t *w, bridge_system_t **list, module_t *m,
    declared_type_t *c)
{
    uint32_t i = 0;

    while (i < CHAIN_SYSTEMS) {
        list[i] = make_system(m, c, i);
        if (!list[i])
            return (false);
        if (i)
            schedule_join_before(list[i], list[i - 1]);
        i++;
    }
    w->systems = list;
    w->system_count = CHAIN_SYSTEMS;
    w->build_in = calloc(CHAIN_SYSTEMS, sizeof(*w->build_in));
    w->build_bits = calloc(CHAIN_SYSTEMS * 2 + 1, sizeof(*w->build_bits));
    return (w->build_in && w->build_bits);
}

static void free_all(bridge_world_t *w, bridge_system_t **list)
{
    uint32_t i = 0;

    while (i < CHAIN_SYSTEMS) {
        free(list[i]);
        i++;
    }
    free(w->build_in);
    free(w->build_bits);
}

enum storage_test_end storage_test_count_levels([[maybe_unused]] storage_test_t *t)
{
    bridge_system_t *list[CHAIN_SYSTEMS] = {};
    char update[] = "update";
    char *phases[1] = { update };
    declared_type_t c = { .name = update };
    module_t m = {};
    bridge_world_t w = { .phases = phases, .phase_count = 1 };
    char why[256] = "";
    bool made = fill_world(&w, list, &m, &c);
    int refused = made ? counted_build(&w, why, sizeof(why)) : 1;

    printf("levels: %u systems in one phase, %s: set_levels tested reach %llu "
        "times, last level %u\n", CHAIN_SYSTEMS,
        refused ? "refused" : "built", (unsigned long long)reach_tests,
        made ? list[CHAIN_SYSTEMS - 1]->level : 0);
    free_all(&w, list);
    return (STORAGE_TEST_DONE);
}
