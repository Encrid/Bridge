/*
** Copyright (C) 2026 Bridge Owner
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/host/budget_reach.c -- budgets that no offset reaches, and counts
** that are used up by the first module.
**
** budgets.md, *Budgets*
*/

#include <stdlib.h>

#include "cases.h"

/*
** test_leave_terms_unset
**
** A deployment that states no count of terms.
*/
void test_leave_terms_unset(host_t *h)
{
    h->cfg.budget.terms = 0;
    host_open_only(h);
}

/*
** test_state_beyond_reach
**
** A size no offset reaches.
*/
void test_state_beyond_reach(host_t *h)
{
    h->cfg.budget.entities = (uint64_t)1 << 40;
    host_open_only(h);
}

/*
** test_state_too_big
**
** Two sizes that are each in reach and together are not.
*/
void test_state_too_big(host_t *h)
{
    h->cfg.budget.persistent = (uint64_t)3 << 30;
    h->cfg.budget.columns = (uint64_t)2 << 30;
    host_open_only(h);
}

/*
** test_use_up_types
**
** Room for one type: the mover declares two components.
*/
void test_use_up_types(host_t *h)
{
    h->cfg.budget.types = 1;
    if (host_open(h))
        return;
    test_load_mover(h);
    host_close(h);
}

/*
** test_use_up_systems
**
** Room for one system: the mover registers two.
*/
void test_use_up_systems(host_t *h)
{
    h->cfg.budget.systems = 1;
    if (host_open(h))
        return;
    test_load_mover(h);
    host_close(h);
}
