/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "cases.h"

static void load_settings(host_t *h, int64_t persistent)
{
    bridge_artifact_t a;

    test_state_configured(h, &a, "game:settings@0.1.0");
    a.cfg.persistent = persistent;
    host_load(h, &a);
}

void test_keep_two_shares(host_t *h)
{
    bridge_setting_t *path = &test_get_noted(h)->settings[5];
    bridge_artifact_t a;

    h->cfg.budget.persistent = 1024;
    if (host_open(h))
        return;
    test_state_guarded_minting(&a, "game:other@0.1.0", "settings");
    host_load(h, &a);
    load_settings(h, 128);
    host_unload(h, "game:settings@0.1.0");
    path->v.s = "a/path/that/is/longer/than/the/first/one.cfg";
    load_settings(h, 128);
    host_unload(h, "game:settings@0.1.0");
    load_settings(h, 128);
    host_unload(h, "game:settings@0.1.0");
    host_unload(h, "game:other@0.1.0");
    load_settings(h, 1024);
    test_say_modules(h);
    host_close(h);
}
