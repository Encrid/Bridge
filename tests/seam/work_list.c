/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdlib.h>
#include <string.h>

#include "cases.h"

static void load_pair(host_t *h, const char *lurker)
{
    bridge_artifact_t a;

    group_state(&a, "game:twoears@0.1.0", "twoears");
    host_load(h, &a);
    group_state(&a, "game:lurker@0.1.0", lurker);
    host_load(h, &a);
}

void test_rows_of_idle(host_t *h)
{
    int frames = atoi(h->arg);

    if (host_open(h))
        return;
    load_pair(h, "lurker_rows");
    for (int i = 0; i < frames; i++) {
        (void)bridge_run_phase(h->w, "update");
        (void)bridge_run_phase(h->w, "present");
    }
    host_close(h);
}

void test_kept_triggers(host_t *h)
{
    if (host_open(h))
        return;
    load_pair(h, "lurker");
    (void)bridge_run_phase(h->w, "update");
    if (!strcmp(h->arg, "present"))
        (void)bridge_run_phase(h->w, "present");
    host_unload(h, "game:lurker@0.1.0");
    host_close(h);
}
