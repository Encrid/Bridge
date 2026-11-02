/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdlib.h>

#include "cases.h"

void test_leave_flag_unset(host_t *h)
{
    char identity[64];
    bridge_artifact_t a;

    if (host_open(h))
        return;
    test_state_small(&a, identity, "ok");
    a.cfg.pinned = (enum bridge_flag)strtol(h->arg, nullptr, 10);
    host_load(h, &a);
    test_state_small(&a, identity, "ok");
    a.cfg.hot_reload = (enum bridge_flag)3;
    host_load(h, &a);
    host_close(h);
}
