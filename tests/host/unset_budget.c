/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge-ASM700.
**
** All rights reserved.
**
** tests/host/unset_budget.c -- keys of the deployment nobody stated: a
** bridge is refused for each.
**
** budgets.md, *Bridge-ASM700 defaults none of them*
*/

#include "cases.h"

/*
** test_leave_phases_unset
**
** A deployment that declares no phase.
*/
void test_leave_phases_unset(host_t *h)
{
    h->cfg.phase_count = 0;
    host_open_only(h);
}

/*
** test_leave_columns_unset
**
** A deployment that states no column region.
*/
void test_leave_columns_unset(host_t *h)
{
    h->cfg.budget.columns = 0;
    host_open_only(h);
}

/*
** test_leave_tables_unset
**
** A deployment that states no count of tables.
*/
void test_leave_tables_unset(host_t *h)
{
    h->cfg.budget.tables = 0;
    host_open_only(h);
}

/*
** test_leave_types_unset
**
** A deployment that states no count of types.
*/
void test_leave_types_unset(host_t *h)
{
    h->cfg.budget.types = 0;
    host_open_only(h);
}

/*
** test_leave_systems_unset
**
** A deployment that states no count of systems.
*/
void test_leave_systems_unset(host_t *h)
{
    h->cfg.budget.systems = 0;
    host_open_only(h);
}
