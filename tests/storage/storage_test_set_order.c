/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>
#include <stdlib.h>

#include "schedule.h"
#include "storage_test.h"

static const bridge_order_t BASE[2] = {
    { "game:m:m2", "game:n:n1" },
    { "game:m:m1", "game:n:n2" }
};

static const bridge_order_t ALT[2] = {
    { "game:n:n1", "game:m:m2" },
    { "game:m:m1", "game:n:n2" }
};

static const bridge_order_t LOOP[2] = {
    { "game:m:m2", "game:n:n1" },
    { "game:n:n1", "game:m:m2" }
};

static const char *const FULLS[4] = {
    "game:m:m1", "game:m:m2", "game:n:n1", "game:n:n2"
};

static char PHASE_UPDATE[] = "update";
static char *PHASES[1] = { PHASE_UPDATE };

typedef struct {
    declared_type_t a;
    declared_type_t b;
    declared_type_t c;
    declared_type_t d;
    module_t m;
    module_t n;
} order_world_t;

static bridge_system_t *make_sys(module_t *m, declared_type_t *own,
    declared_type_t *other, bool writes)
{
    bridge_system_t *s = calloc(1, sizeof(*s) + 2 * sizeof(system_term_t)
        + sizeof(uint64_t));

    if (!s)
        return (nullptr);
    s->m = m;
    s->terms = 2;
    s->term[0] = (system_term_t){ own, true };
    s->term[1] = (system_term_t){ other, writes };
    s->stated_before = (uint64_t *)(void *)&s->term[2];
    return (s);
}

static bool make_all(order_world_t *o, bridge_system_t **s)
{
    s[0] = make_sys(&o->m, &o->a, &o->d, false);
    s[1] = make_sys(&o->m, &o->a, &o->c, true);
    s[2] = make_sys(&o->n, &o->b, &o->c, false);
    s[3] = make_sys(&o->n, &o->b, &o->d, true);
    if (!s[0] || !s[1] || !s[2] || !s[3])
        return (false);
    s[1]->slot = 1;
    s[3]->slot = 1;
    schedule_join_before(s[1], s[0]);
    schedule_join_before(s[3], s[2]);
    for (uint32_t k = 0; k < 4; k++)
        s[k]->full = (char *)FULLS[k];
    return (true);
}

static void report(const char *what, bridge_system_t **s, int refused)
{
    printf("%s: %s", what, refused ? "refused" : "ok");
    for (uint32_t k = 0; k < 4; k++)
        printf(" %u", s[k]->level);
    printf("\n");
}

static void tidy(bridge_world_t *w, bridge_system_t **s)
{
    for (uint32_t k = 0; k < 4; k++)
        free(s[k]);
    schedule_free_pairs(&w->order);
    free(w->build_in);
    free(w->build_bits);
    free(w->run_jobs);
    free(w->run_lists);
}

enum storage_test_end storage_test_set_order(storage_test_t *t)
{
    char cn[4][2] = { "a", "b", "c", "d" };
    order_world_t o = { .a = { .name = cn[0] }, .b = { .name = cn[1] },
        .c = { .name = cn[2] }, .d = { .name = cn[3] } };
    bridge_system_t *s[4] = {};
    bridge_world_t w = { .phases = PHASES, .phase_count = 1,
        .sink = storage_test_hear, .ud = t };
    char why[256] = "";

    w.build_in = calloc(4, sizeof(*w.build_in));
    w.build_bits = calloc(4 * 2 + 1, sizeof(*w.build_bits));
    w.run_jobs = calloc(4, sizeof(schedule_job_t));
    w.run_lists = calloc(4, sizeof(*w.run_lists));
    w.systems = s;
    w.system_count = 4;
    if (!w.build_in || !w.build_bits || !w.run_jobs || !w.run_lists
        || !make_all(&o, s)
        || schedule_order(&w, BASE, 2) || schedule_build(&w, why, sizeof(why)))
        return (STORAGE_TEST_FAILED);
    report("cycle", s, bridge_set_order(&w, LOOP, 2));
    report("empty", s, bridge_set_order(&w, nullptr, 0));
    report("alt", s, bridge_set_order(&w, ALT, 2));
    tidy(&w, s);
    return (STORAGE_TEST_DONE);
}
