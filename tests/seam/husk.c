/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cases.h"

static unsigned long long count_entities(host_t *h)
{
    bridge_budget_t used = {};

    (void)bridge_usage(h->w, &used, nullptr);
    return ((unsigned long long)used.entities);
}

static void state_husk(bridge_artifact_t *a, const char *identity,
    const char *file)
{
    test_state_guarded_minting(a, identity, file);
    a->cfg.spawns = 3;
    a->cfg.reloadable = BRIDGE_TRUE;
    a->cfg.hot_reload = BRIDGE_TRUE;
}

static void load_husk(host_t *h, const char *file)
{
    bridge_artifact_t a;

    state_husk(&a, "husk:husk@0.1.0", file);
    host_load(h, &a);
}

static void run_script(host_t *h)
{
    unsigned long long before;

    host_run_frames(h, 1);
    before = count_entities(h);
    host_run_frames(h, 1);
    printf("entities freed by the destroys: %llu\n",
        before - count_entities(h));
    host_run_frames(h, 2);
    before = count_entities(h);
    host_run_frames(h, 1);
    printf("entities freed by the detach: %llu\n",
        before - count_entities(h));
}

void test_make_husk(host_t *h)
{
    if (host_open(h))
        return;
    load_husk(h, h->arg);
    run_script(h);
    host_close(h);
}

void test_reload_husk(host_t *h)
{
    bridge_artifact_t a;

    if (host_open(h))
        return;
    load_husk(h, h->arg);
    run_script(h);
    state_husk(&a, "husk:husk@0.2.0", h->arg);
    host_reload(h, &a);
    host_run_frames(h, 1);
    host_close(h);
}

void test_unload_husk(host_t *h)
{
    bool twin = !strcmp(h->arg, "twin");

    if (host_open(h))
        return;
    if (twin)
        test_load_spawnless(h, "husk:twin@0.1.0", "husk-twin");
    load_husk(h, twin ? "husk" : h->arg);
    run_script(h);
    host_unload(h, "husk:husk@0.1.0");
    printf("entities after the unload: %llu\n", count_entities(h));
    host_close(h);
}

void test_crowd_husk(host_t *h)
{
    h->cfg.budget.tables = (uint32_t)atoi(h->arg);
    if (host_open(h))
        return;
    load_husk(h, "husk");
    host_run_frames(h, 2);
    host_close(h);
}
