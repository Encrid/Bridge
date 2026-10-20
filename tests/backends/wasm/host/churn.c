/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>

#include "cases.h"

static int churn_one(host_t *h, const bridge_artifact_t *a)
{
    int refused = bridge_load(h->w, a);

    host_run_frames(h, 1);
    (void)bridge_unload(h->w, a->cfg.identity);
    return (refused);
}

static int churn_three(host_t *h)
{
    bridge_artifact_t a;
    int refused;

    test_state_wasm_minting(&a, "game:nested@0.1.0", "nested");
    a.cfg.spawns = 8;
    refused = churn_one(h, &a);
    test_state_configured(h, &a, "game:settings@0.1.0");
    refused += churn_one(h, &a);
    test_state_wasm_minting(&a, "game:keeper@0.1.0", "keeper-sweep");
    return (refused + churn_one(h, &a));
}

void test_churn(host_t *h)
{
    test_noted_t *more = test_get_noted(h);
    int refused = 0;
    int i = 0;

    h->cfg.budget.persistent = 64;
    if (host_open(h))
        return;
    more->quiet = true;
    while (i < 200) {
        refused += churn_three(h);
        i++;
    }
    more->quiet = false;
    printf("churned 200 times, %d loads refused, %u records of the index "
        "out of step\n", refused, more->out_of_step);
    test_say_modules(h);
    host_close(h);
}

void test_drop_configured(host_t *h)
{
    if (host_open(h))
        return;
    test_reload_settings(h, true);
    test_load_spawnless(h, "game:retype@0.1.0", "retype-settings");
    host_close(h);
}
