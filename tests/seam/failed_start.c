/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <string.h>

#include "cases.h"

static void state_why(bridge_artifact_t *a, const char *identity,
    const char *file)
{
    host_state_guarded(a, identity, file);
    a->cfg.reloadable = BRIDGE_TRUE;
    a->cfg.hot_reload = BRIDGE_TRUE;
}

void test_drain_failed_start(host_t *h)
{
    bridge_artifact_t a;

    if (host_open(h))
        return;
    if (!strcmp(h->arg, "init")) {
        state_why(&a, "why:init@0.1.0", "why");
        host_load(h, &a);
    } else {
        state_why(&a, "why:reload@0.1.0", "why-reload");
        host_load(h, &a);
        state_why(&a, "why:reload@0.2.0", "why-reload");
        host_reload(h, &a);
    }
    test_say_modules(h);
    host_close(h);
}

void test_shut_failed_load(host_t *h)
{
    bridge_artifact_t a;

    if (host_open(h))
        return;
    state_why(&a, "why:settings@0.1.0", h->arg);
    a.cfg.settings = TEST_STRAY;
    a.cfg.setting_count = 1;
    host_load(h, &a);
    test_say_modules(h);
    host_close(h);
}

void test_keep_after_failed_reload(host_t *h)
{
    bridge_artifact_t a;

    if (host_open(h))
        return;
    test_load_guarded(h, "game:mover@0.1.0", "mover");
    state_why(&a, "game:keep@0.1.0", "keep");
    host_load(h, &a);
    state_why(&a, "game:keep@0.2.0", "keep-fail");
    host_reload(h, &a);
    host_run_frames(h, 1);
    test_say_modules(h);
    host_close(h);
}
