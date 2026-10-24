/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <string.h>

#include "cases.h"

static void state_twice(const host_t *h, bridge_artifact_t *a,
    const char *identity)
{
    host_state_plain(a, identity, nullptr);
    host_name_module(a, strstr(h->arg, "marked") ? "reload-takes-marked"
        : "reload-takes");
    if (!strcmp(h->arg, "guarded"))
        host_state_guarded(a, identity, "reload-takes");
    a->cfg.reloadable = BRIDGE_TRUE;
    a->cfg.hot_reload = BRIDGE_TRUE;
    a->cfg.spawns = 4;
    a->cfg.persistent = 64;
}

void test_take_in_old_shutdown(host_t *h)
{
    bridge_artifact_t a;

    if (host_open(h))
        return;
    state_twice(h, &a, "dup:twice@0.1.0");
    host_load(h, &a);
    host_run_frames(h, 1);
    state_twice(h, &a, "dup:twice@0.2.0");
    host_reload(h, &a);
    host_run_frames(h, 1);
    test_say_modules(h);
    host_close(h);
}
