/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "cases.h"

static void run_beside_hold(host_t *h, const char *identity)
{
    bridge_artifact_t a;

    if (host_open(h))
        return;
    host_state_plain(&a, "test:hold@0.1.0", nullptr);
    host_name_module(&a, "hold");
    a.cfg.spawns = 4;
    host_load(h, &a);
    host_state_plain(&a, identity, nullptr);
    host_name_module(&a, h->arg);
    a.cfg.changes = 1024;
    host_load(h, &a);
    host_run_frames(h, 2);
    host_close(h);
}

void test_guess(host_t *h)
{
    run_beside_hold(h, "test:guess@0.1.0");
}

void test_stick(host_t *h)
{
    run_beside_hold(h, "test:stick@0.1.0");
}
