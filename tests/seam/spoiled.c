/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>
#include <string.h>

#include "cases.h"
#include "host_copy.h"

static void call(host_t *h, const bridge_artifact_t *a, bool reload,
    void **spoiled)
{
    bool before = !strcmp(h->arg, "before");
    host_copy_t c;
    const bridge_artifact_t *copy = host_copy(&c, a, 1);
    int refused = 0;

    if (before)
        host_spoil(spoiled, &c);
    if (reload)
        refused = bridge_reload(h->w, copy, 1);
    else
        refused = bridge_load(h->w, copy);
    if (!before)
        host_spoil(spoiled, &c);
    printf("%s: %s%s\n", a->cfg.identity, refused ? "not " : "",
        reload ? "reloaded" : "loaded");
}

void test_spoil_config(host_t *h)
{
    static const bridge_phase_name_t map = { "update", "present" };
    void *spoiled = nullptr;
    bridge_artifact_t a;

    if (host_open(h))
        return;
    test_load_mover(h);
    test_state_reader(&a);
    call(h, &a, false, &spoiled);
    test_state_configured(h, &a, "game:settings@0.1.0");
    a.cfg.phases = &map;
    a.cfg.phase_count = 1;
    a.cfg.hot_reload = BRIDGE_TRUE;
    a.cfg.reloadable = BRIDGE_TRUE;
    call(h, &a, false, &spoiled);
    host_run_frames(h, 1);
    a.cfg.identity = "game:settings@0.2.0";
    call(h, &a, true, &spoiled);
    host_run_frames(h, 1);
    host_close(h);
    host_free_spoiled(&spoiled);
}
