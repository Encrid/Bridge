/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdlib.h>

#include "cases.h"

void test_leave_terms_unset(host_t *h)
{
    h->cfg.budget.terms = 0;
    host_open_only(h);
}

void test_state_beyond_reach(host_t *h)
{
    h->cfg.budget.entities = (uint64_t)1 << 40;
    host_open_only(h);
}

void test_state_too_big(host_t *h)
{
    h->cfg.budget.persistent = (uint64_t)3 << 30;
    h->cfg.budget.columns = (uint64_t)2 << 30;
    host_open_only(h);
}

void test_use_up_types(host_t *h)
{
    h->cfg.budget.types = 1;
    if (host_open(h))
        return;
    test_load_mover(h);
    host_close(h);
}

void test_use_up_systems(host_t *h)
{
    h->cfg.budget.systems = 1;
    if (host_open(h))
        return;
    test_load_mover(h);
    host_close(h);
}
