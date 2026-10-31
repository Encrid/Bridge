/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "storage_test.h"

static const bridge_order_t ORDER_PAIRS[2] = {
    { "game:m:m2", "game:n:n1" },
    { "game:n:n2", "game:m:m1" }
};

static const bridge_order_t CHAIN_PAIRS[2] = {
    { "game:m:m2", "game:n:n1" },
    { "game:m:m1", "game:n:n2" }
};

static const char *const ORDER_NAMES[4] = {
    "game:m:m1", "game:m:m2", "game:n:n1", "game:n:n2"
};

typedef struct {
    declared_type_t a;
    declared_type_t b;
    declared_type_t c;
    declared_type_t d;
    module_t m;
    module_t n;
} order_world_t;

static bridge_system_t *make_two(module_t *m, declared_type_t *own,
    declared_type_t *other, bool writes)
{
    bridge_system_t *s = storage_test_make_system(m, 2, 0, 1);

    if (!s)
        return (nullptr);
    s->term[0] = (system_term_t){ own, true };
    s->term[1] = (system_term_t){ other, writes };
    schedule_index_system(s);
    return (s);
}

static bool make_four(order_world_t *o, bridge_system_t **s)
{
    s[0] = make_two(&o->m, &o->a, &o->d, false);
    s[1] = make_two(&o->m, &o->a, &o->c, true);
    s[2] = make_two(&o->n, &o->b, &o->c, false);
    s[3] = make_two(&o->n, &o->b, &o->d, true);
    if (!s[0] || !s[1] || !s[2] || !s[3])
        return (false);
    s[1]->slot = 1;
    s[3]->slot = 1;
    schedule_join_before(s[1], s[0]);
    schedule_join_before(s[3], s[2]);
    return (true);
}

static void free_world(bridge_world_t *w, bridge_system_t **s)
{
    uint32_t k = 0;

    while (k < 4) {
        free(s[k]);
        k++;
    }
    schedule_free_pairs(&w->order);
    free(w->build_in);
    free(w->build_bits);
    free(w->run_jobs);
    free(w->run_lists);
}

static void build_in_order(bridge_world_t *w, bridge_system_t **s,
    const uint32_t *order, const char *what)
{
    bridge_system_t *list[4];
    char why[256] = "";
    uint32_t k = 0;
    int refused;

    while (k < 4) {
        list[k] = s[order[k]];
        list[k]->level = 0;
        k++;
    }
    w->systems = list;
    w->system_count = 4;
    refused = storage_test_build_all(w, why, sizeof(why));
    printf("%s: %s%s\n", what, refused ? "refused: " : "built", why);
    k = 0;
    while (!refused && k < 4) {
        printf("  %s at level %u\n", ORDER_NAMES[k], s[k]->level);
        k++;
    }
}

static void build_both(bridge_world_t *w, bridge_system_t **s, const char *what,
    uint32_t barrier)
{
    static const uint32_t M_FIRST[4] = { 0, 1, 2, 3 };
    static const uint32_t N_FIRST[4] = { 2, 3, 0, 1 };
    static const uint32_t M_REVERSED[4] = { 1, 0, 3, 2 };
    static const uint32_t N_REVERSED[4] = { 3, 2, 1, 0 };

    if (!strcmp(what, "orderreversed")) {
        build_in_order(w, s, M_REVERSED,
            "game:m loaded first, the systems of each module reversed");
        build_in_order(w, s, N_REVERSED,
            "game:n loaded first, the systems of each module reversed");
        return;
    }
    build_in_order(w, s, M_FIRST, barrier
        ? "game:m loaded first, at the barrier" : "game:m loaded first");
    build_in_order(w, s, N_FIRST, barrier
        ? "game:n loaded first, at the barrier" : "game:n loaded first");
}

enum storage_test_end storage_test_order_two_modules(storage_test_t *t)
{
    char update[] = "update";
    char *phases[1] = { update };
    char names[4][8] = { "game:A", "game:B", "game:C", "game:D" };
    char fulls[4][16] = { "game:m:m1", "game:m:m2", "game:n:n1",
        "game:n:n2" };
    order_world_t o = { .a = { .name = names[0] }, .b = { .name = names[1] },
        .c = { .name = names[2] }, .d = { .name = names[3] } };
    bridge_system_t *s[4] = {};
    bridge_world_t w = { .phases = phases, .phase_count = 1 };
    bool cycle = !strcmp(t->what, "ordertwo")
        || !strcmp(t->what, "ordercyclebarrier");
    uint32_t barrier = !strcmp(t->what, "orderbarrier")
        || !strcmp(t->what, "ordercyclebarrier");
    uint32_t k = 0;

    w.build_in = calloc(4, sizeof(*w.build_in));
    w.build_bits = calloc(4 * 2 + 1, sizeof(*w.build_bits));
    w.run_jobs = calloc(4, sizeof(schedule_job_t));
    w.run_lists = calloc(4, sizeof(*w.run_lists));
    if (!w.build_in || !w.build_bits || !w.run_jobs || !w.run_lists
        || !make_four(&o, s)
        || schedule_order(&w, cycle ? ORDER_PAIRS : CHAIN_PAIRS, 2))
        printf("FAIL: the world of the order was not made\n");
    while (k < 4 && s[k]) {
        s[k]->full = fulls[k];
        s[k]->named = schedule_find_name(&w.order, fulls[k]);
        s[k]->phase = barrier;
        s[k]->observer = (int)barrier;
        k++;
    }
    build_both(&w, s, t->what, barrier);
    free_world(&w, s);
    return (STORAGE_TEST_DONE);
}
