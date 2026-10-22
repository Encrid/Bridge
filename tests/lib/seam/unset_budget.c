/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "cases.h"

void test_leave_phases_unset(host_t *h)
{
    h->cfg.phase_count = 0;
    host_open_only(h);
}

void test_leave_columns_unset(host_t *h)
{
    h->cfg.budget.columns = 0;
    host_open_only(h);
}

void test_leave_tables_unset(host_t *h)
{
    h->cfg.budget.tables = 0;
    host_open_only(h);
}

void test_leave_types_unset(host_t *h)
{
    h->cfg.budget.types = 0;
    host_open_only(h);
}

void test_leave_systems_unset(host_t *h)
{
    h->cfg.budget.systems = 0;
    host_open_only(h);
}
