/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>
#include <string.h>

#include "cases.h"

void group_state(bridge_artifact_t *a, const char *identity, const char *file)
{
    test_state_guarded(a, identity, file);
    a->cfg.reloadable = BRIDGE_TRUE;
    a->cfg.hot_reload = BRIDGE_TRUE;
    a->cfg.spawns = 8;
}

void test_reload_group(host_t *h)
{
    char to[2][32] = { "", "" };
    char third[32] = "";
    bridge_artifact_t a[3];

    (void)sscanf(h->arg, "%31s %31s %31s", to[0], to[1], third);
    if (host_open(h))
        return;
    group_state(&a[0], "game:one@0.1.0", "share1");
    group_state(&a[1], "game:two@0.1.0", "share1");
    group_state(&a[2], "game:three@0.1.0", "share1");
    host_load(h, &a[0]);
    host_load(h, &a[1]);
    if (!strcmp(third, "three"))
        host_load(h, &a[2]);
    host_run_frames(h, 1);
    group_state(&a[0], "game:one@0.2.0", to[0]);
    group_state(&a[1], "game:two@0.2.0", to[1]);
    host_reload_many(h, a, 2);
    host_run_frames(h, 1);
    test_say_modules(h);
    host_close(h);
}
