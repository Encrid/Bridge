/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>

#include "cases.h"

void test_breach(host_t *h)
{
    bridge_artifact_t a;

    h->cfg.budget.sparse_sets = 8;
    h->cfg.budget.direct_indexed = 8;
    if (host_open(h))
        return;
    host_state_wasm(&a, "game:breach@0.1.0", h->arg);
    a.cfg.spawns = 32;
    a.cfg.persistent = 256;
    host_load(h, &a);
    host_run_frames(h, 1);
    host_close(h);
}

void test_hoard(host_t *h)
{
    bridge_artifact_t a;
    char identity[32];
    uint32_t v = 2;

    if (host_open(h))
        return;
    test_state_ticker(&a, "game:ticker@0.1.0", "ticker");
    a.cfg.persistent = 64;
    host_load(h, &a);
    test_state_wasm_minting(&a, "game:namer@0.1.0", "namer");
    a.cfg.persistent = 64;
    host_load(h, &a);
    host_run_frames(h, 1);
    while (v <= 9) {
        snprintf(identity, sizeof(identity), "game:ticker@0.%u.0", v);
        test_state_ticker(&a, identity, "ticker");
        a.cfg.persistent = 64 * v;
        host_reload(h, &a);
        v++;
    }
    host_run_frames(h, 2);
    host_close(h);
}
