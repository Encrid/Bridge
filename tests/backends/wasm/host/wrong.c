/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "cases.h"

static const char *const TEST_THING[] = {
    "other:Thing"
};

void test_do_wrong(host_t *h)
{
    bridge_artifact_t a;

    if (host_open(h))
        return;
    host_state_wasm(&a, "game:wrong@0.1.0", h->arg);
    a.cfg.read = TEST_THING;
    a.cfg.read_count = 1;
    host_load(h, &a);
    host_run_frames(h, 1);
    host_close(h);
}
