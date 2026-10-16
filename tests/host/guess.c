/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "cases.h"

void test_guess(host_t *h)
{
    bridge_artifact_t a;

    if (host_open(h))
        return;
    host_state_plain(&a, "test:hold@0.1.0", nullptr);
    host_name_module(&a, "hold");
    a.cfg.spawns = 4;
    host_load(h, &a);
    host_state_plain(&a, "test:guess@0.1.0", nullptr);
    host_name_module(&a, h->arg);
    a.cfg.changes = 1024;
    host_load(h, &a);
    host_run_frames(h, 2);
    host_close(h);
}
