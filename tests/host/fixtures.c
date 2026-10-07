/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/host/fixtures.c -- the fixtures of the host program: what a case's
** host is before the case changes it.
*/

#include "cases.h"

/*
** test_set_small
**
** The wasm backend, the one phase "update", and the sink that prints a
** record's level.
*/
void test_set_small(host_t *h)
{
    h->cfg.backend_count = 1;
    h->cfg.phase_count = 1;
    h->sink = test_print;
}

/*
** test_set_wasm
**
** The wasm backend and both phases.
*/
void test_set_wasm(host_t *h)
{
    h->cfg.backend_count = 1;
    h->sink = host_print_record;
}

/*
** test_set_both
**
** Both backends and both phases.
*/
void test_set_both(host_t *h)
{
    h->sink = host_print_record;
}

/*
** test_set_counting
**
** Both backends, both phases, and the sink that counts.
*/
void test_set_counting(host_t *h)
{
    test_kept_t *kept = h->kept;

    h->sink = test_hear_counts;
    h->ud = &kept->listener;
}

/*
** test_set_noting
**
** Both backends, both phases, and the sink that notes.
*/
void test_set_noting(host_t *h)
{
    test_kept_t *kept = h->kept;

    h->sink = test_hear_noted;
    h->ud = &kept->noted;
}
