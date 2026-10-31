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

void counted_set_levels(schedule_phase_t *p);

#define schedule_reaches count_reach
#define schedule_set_levels counted_set_levels
#include "schedule_levels.c"
#undef schedule_set_levels
#undef schedule_reaches

static bool fill_world(bridge_world_t *w, bridge_system_t **list, module_t *m,
    declared_type_t *c)
{
    uint32_t i = 0;

    while (i < CHAIN_SYSTEMS) {
        list[i] = storage_test_make_system(m, 1, 0, bits_words(i));
        if (!list[i])
            return (false);
        list[i]->term[0] = (system_term_t){ c, true };
        list[i]->slot = i;
        schedule_index_system(list[i]);
        if (i)
            schedule_join_before(list[i], list[i - 1]);
        i++;
    }
    w->systems = list;
    w->system_count = CHAIN_SYSTEMS;
    w->build_in = calloc(CHAIN_SYSTEMS, sizeof(*w->build_in));
    w->build_bits = calloc(CHAIN_SYSTEMS * 2 + 1, sizeof(*w->build_bits));
    w->run_jobs = calloc(CHAIN_SYSTEMS, sizeof(schedule_job_t));
    w->run_lists = calloc(CHAIN_SYSTEMS, sizeof(*w->run_lists));
    return (w->build_in && w->build_bits && w->run_jobs && w->run_lists);
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
    free(w->run_jobs);
    free(w->run_lists);
}

static void count_levels(bridge_world_t *w)
{
    schedule_phase_t p = { .order = &w->order,
        .in = (bridge_system_t **)w->build_in };

    schedule_mark_all(w);
    schedule_gather_group(w, &p);
    schedule_add_stated(w, &p);
    schedule_add_own(&p);
    counted_set_levels(&p);
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
    int refused = made ? storage_test_build_all(&w, why, sizeof(why)) : 1;

    if (!refused)
        count_levels(&w);
    printf("levels: %u systems in one phase, %s: set_levels tested reach %llu "
        "times, last level %u\n", CHAIN_SYSTEMS,
        refused ? "refused" : "built", (unsigned long long)reach_tests,
        made ? list[CHAIN_SYSTEMS - 1]->level : 0);
    free_all(&w, list);
    return (STORAGE_TEST_DONE);
}
