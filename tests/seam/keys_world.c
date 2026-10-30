/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cases.h"

void test_pass_chain(host_t *h)
{
    h->cfg.observer_passes = (uint32_t)atoi(h->arg);
    if (host_open(h))
        return;
    test_load_guarded(h, "game:chain@0.1.0", "chain");
    (void)bridge_run_phase(h->w, "update");
    printf("one call\n");
    host_close(h);
}

void test_fill_tables(host_t *h)
{
    h->cfg.table_hash_fill = 1;
    h->cfg.budget.tables = (uint32_t)strtoul(h->arg, nullptr, 10);
    host_open_only(h);
}

void test_grow_elements(host_t *h)
{
    char identity[64];
    bridge_artifact_t a;

    if (host_open(h))
        return;
    test_state_small(&a, identity, "grow_elements");
    if (!strcmp(h->arg, "unbounded"))
        a.cfg.element_limit = BRIDGE_NO_LIMIT;
    host_load(h, &a);
    host_close(h);
}
