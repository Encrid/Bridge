/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "storage_test.h"

static char FA[] = "game:m:a";
static char FB[] = "game:n:b";

static const bridge_order_t PHASE_PAIR[1] = {
    { FA, FB }
};

static const bridge_order_t SAME_PAIRS[3] = {
    { FA, FB },
    { FA, "game:x:gone" },
    { "game:x:gone", FB }
};

typedef struct {
    declared_type_t a;
    declared_type_t b;
    module_t m;
    module_t n;
} phase_world_t;

static bridge_system_t *make_sys(module_t *m, declared_type_t *c)
{
    bridge_system_t *s = calloc(1, sizeof(*s) + sizeof(system_term_t));

    if (s) {
        s->m = m;
        s->terms = 1;
        s->term[0] = (system_term_t){ c, true };
    }
    return (s);
}

static bool setup(phase_world_t *o, bridge_system_t **s, bridge_world_t *w,
    bool cross)
{
    w->build_in = calloc(4, sizeof(*w->build_in));
    w->build_bits = calloc(9, sizeof(*w->build_bits));
    w->run_jobs = calloc(4, sizeof(schedule_job_t));
    w->run_lists = calloc(4, sizeof(*w->run_lists));
    s[0] = make_sys(&o->m, &o->a);
    s[1] = make_sys(&o->n, &o->b);
    if (!w->build_in || !w->build_bits || !w->run_jobs || !w->run_lists
        || !s[0] || !s[1]
        || schedule_order(w, cross ? PHASE_PAIR : SAME_PAIRS, cross ? 1 : 3))
        return (false);
    s[0]->full = FA;
    s[1]->full = FB;
    s[0]->named = schedule_find_name(&w->order, FA);
    s[1]->named = schedule_find_name(&w->order, FB);
    s[1]->phase = cross;
    s[1]->slot = 1;
    w->systems = s;
    w->system_count = 2;
    return (true);
}

static void free_phase(bridge_world_t *w, bridge_system_t **s)
{
    free(s[0]);
    free(s[1]);
    schedule_free_pairs(&w->order);
    free(w->build_in);
    free(w->build_bits);
    free(w->run_jobs);
    free(w->run_lists);
}

enum storage_test_end storage_test_order_phase(storage_test_t *t)
{
    phase_world_t o = {};
    char up[] = "update";
    char late[] = "late";
    char *phases[2] = { up, late };
    bridge_world_t w = { .phases = phases, .phase_count = 2 };
    bridge_system_t *s[2] = {};
    char why[256] = "";
    int refused;

    if (!setup(&o, s, &w, !strcmp(t->what, "orderphases")))
        printf("FAIL: the world of the phases was not made\n");
    refused = schedule_build(&w, why, sizeof(why));
    printf("%s: %s%s\n", t->what, refused ? "refused: " : "built", why);
    free_phase(&w, s);
    return (STORAGE_TEST_DONE);
}
