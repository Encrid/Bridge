/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdlib.h>

#include "cases.h"

static void load_namer(host_t *h, int64_t persistent)
{
    bridge_artifact_t a;

    if (host_open(h))
        return;
    test_state_module(&a, "game:namer@0.1.0", "namer");
    a.cfg.persistent = persistent;
    host_load(h, &a);
    host_run_frames(h, 1);
    host_close(h);
}

void test_keep_string(host_t *h)
{
    load_namer(h, strtol(h->arg, nullptr, 10));
}

void test_use_up_terms(host_t *h)
{
    h->cfg.budget.terms = (uint32_t)strtol(h->arg, nullptr, 10);
    if (host_open(h))
        return;
    test_load_reader(h);
    host_close(h);
}
