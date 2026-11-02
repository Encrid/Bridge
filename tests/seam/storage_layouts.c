/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdlib.h>

#include "cases.h"

void test_reach_direct(host_t *h)
{
    bridge_artifact_t a;

    h->cfg.budget.direct_indexed = 1;
    if (host_open(h))
        return;
    test_state_module(&a, "game:aim@0.1.0", h->arg);
    a.cfg.spawns = 16;
    host_load(h, &a);
    host_run_frames(h, 1);
    host_close(h);
}

void test_declare_huge(host_t *h)
{
    h->cfg.budget.direct_indexed = 1;
    h->cfg.budget.entities = strtoull(h->arg, nullptr, 10);
    if (host_open(h))
        return;
    test_load_small(h, "huge-direct");
    host_close(h);
}

void test_grow_past_room(host_t *h)
{
    bridge_artifact_t a;

    if (host_open(h))
        return;
    host_state_plain(&a, "test:huge@0.1.0", nullptr);
    host_name_module(&a, "huge");
    a.cfg.spawns = 2;
    a.cfg.reloadable = BRIDGE_TRUE;
    a.cfg.hot_reload = BRIDGE_TRUE;
    host_load(h, &a);
    host_run_frames(h, 1);
    a.cfg.identity = "test:huge@0.2.0";
    host_name_module(&a, h->arg);
    host_reload(h, &a);
    test_say_modules(h);
    host_close(h);
}

void test_count_toggled(host_t *h)
{
    bridge_artifact_t a;

    if (host_open(h))
        return;
    test_state_module(&a, "game:shield@0.1.0", h->arg);
    a.cfg.spawns = 16;
    host_load(h, &a);
    (void)bridge_run_phase(h->w, "present");
    host_run_frames(h, 1);
    host_close(h);
}

void test_walk_sparse(host_t *h)
{
    bridge_artifact_t a;

    h->cfg.budget.sparse_sets = 1;
    if (host_open(h))
        return;
    test_state_module(&a, "game:hit@0.1.0", h->arg);
    a.cfg.spawns = 16;
    host_load(h, &a);
    host_run_frames(h, 2);
    host_close(h);
}

void test_state_bad_term(host_t *h)
{
    if (host_open(h))
        return;
    test_load_module(h, "game:badterm@0.1.0", h->arg);
    host_close(h);
}

void test_undo_load(host_t *h)
{
    if (host_open(h))
        return;
    test_load_module(h, "game:badterm@0.1.0", h->arg);
    test_load_module(h, "game:fp@0.1.0", "fp");
    host_close(h);
}
