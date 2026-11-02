/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdlib.h>

#include "cases.h"

void test_use_up_types(host_t *h)
{
    h->cfg.budget.types = (uint32_t)strtol(h->arg, nullptr, 10);
    if (host_open(h))
        return;
    test_load_mover(h);
    host_close(h);
}

void test_use_up_systems(host_t *h)
{
    h->cfg.budget.systems = (uint32_t)strtol(h->arg, nullptr, 10);
    if (host_open(h))
        return;
    test_load_mover(h);
    host_close(h);
}
