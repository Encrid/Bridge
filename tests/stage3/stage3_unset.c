/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge-ASM700.
**
** All rights reserved.
**
** tests/stage3/stage3_unset.c -- keys of the deployment nobody stated: a
** bridge is refused for each.
**
** budgets.md, *Bridge-ASM700 defaults none of them*
*/

#include "stage3.h"

/*
** stage3_leave_phases_unset
**
** A deployment that declares no phase.
*/
void stage3_leave_phases_unset(host_t *h)
{
    h->cfg.phase_count = 0;
    host_open_only(h);
}

/*
** stage3_leave_columns_unset
**
** A deployment that states no column region.
*/
void stage3_leave_columns_unset(host_t *h)
{
    h->cfg.budget.columns = 0;
    host_open_only(h);
}

/*
** stage3_leave_tables_unset
**
** A deployment that states no count of tables.
*/
void stage3_leave_tables_unset(host_t *h)
{
    h->cfg.budget.tables = 0;
    host_open_only(h);
}

/*
** stage3_leave_types_unset
**
** A deployment that states no count of types.
*/
void stage3_leave_types_unset(host_t *h)
{
    h->cfg.budget.types = 0;
    host_open_only(h);
}

/*
** stage3_leave_systems_unset
**
** A deployment that states no count of systems.
*/
void stage3_leave_systems_unset(host_t *h)
{
    h->cfg.budget.systems = 0;
    host_open_only(h);
}
