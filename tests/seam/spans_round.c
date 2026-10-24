/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdlib.h>

#include "cases.h"

void test_round_spans(host_t *h)
{
    bridge_artifact_t a;
    char *file;

    h->cfg.budget.tables = 63;
    if (host_open(h))
        return;
    host_state_plain(&a, "test:spare@0.1.0", nullptr);
    a.cfg.spans = (uint32_t)strtoul(h->arg, &file, 10);
    host_name_module(&a, *file ? file : "spare");
    if (*file)
        a.cfg.spawns = 1;
    host_load(h, &a);
    host_run_frames(h, 1);
    test_say_modules(h);
    host_close(h);
}
