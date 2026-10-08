/*
** Copyright (C) 2026 Bridge Owner
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/host/arena.c -- a string a component keeps: allocated
** from the module's share of the persistent arena. And a count of terms
** that a query uses up.
**
** strings.md, *Variable length*
*/

#include "cases.h"

static void load_namer(host_t *h, int64_t persistent)
{
    bridge_artifact_t a;

    if (host_open(h))
        return;
    test_state_module(&a, "game:namer@0.1.0", "namer");
    a.cfg.persistent = persistent;
    host_load(h, &a);
    host_run_frames(h, 1);
    host_close(h);
}

/*
** test_keep_string
**
** The namer, with a share its string fits in.
*/
void test_keep_string(host_t *h)
{
    load_namer(h, 64);
}

/*
** test_keep_string_no_room
**
** The namer, with a share its string does not fit in.
*/
void test_keep_string_no_room(host_t *h)
{
    load_namer(h, 4);
}

/*
** test_keep_string_too_big
**
** The namer, stating a share the deployment's arena does not hold.
*/
void test_keep_string_too_big(host_t *h)
{
    load_namer(h, 1 << 20);
}

/*
** test_use_up_terms
**
** Room for one term: the reader's query has two.
*/
void test_use_up_terms(host_t *h)
{
    h->cfg.budget.terms = 1;
    if (host_open(h))
        return;
    test_load_reader(h);
    host_close(h);
}
