/*
** Copyright (C) 2026 Bridge Owner
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/host/churn.c -- modules whose types are enums and nested
** structs, loaded and unloaded many times; and a configured module that
** drops its component at a reload.
**
** strings.md, *It is given back when nothing points into it*
*/

#include <stdio.h>

#include "cases.h"

/*
** churn_one
**
** Loads a module, runs a frame and unloads it. Answers 1 if the load was
** refused.
*/
static int churn_one(host_t *h, const bridge_artifact_t *a)
{
    int refused = bridge_load(h->b, a);

    host_run_frames(h, 1);
    (void)bridge_unload(h->b, a->cfg.identity);
    return (refused);
}

/*
** churn_three
**
** One round: the nested module, the settings module and the keeper, each
** loaded, run and unloaded. Answers how many loads were refused.
*/
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

/*
** test_churn
**
** Two hundred rounds, with room in the arena for one module's share and no
** more: every load takes it, and every unload gives it back. Each unload
** frees tables, and each load makes new ones where the freed ones were: a
** table id that outlived its table would be handed to the wrong one, and
** the row-count check would say the index is out of step.
*/
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

/*
** test_drop_configured
**
** A reload into an instance that declares nothing: the row goes with the
** component, whose name is free for another layout.
*/
void test_drop_configured(host_t *h)
{
    if (host_open(h))
        return;
    test_reload_settings(h, true);
    test_load_spawnless(h, "game:retype@0.1.0", "retype-settings");
    host_close(h);
}
