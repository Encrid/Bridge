/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "cases.h"

void test_set_small(host_t *h)
{
    h->cfg.phase_count = 1;
    h->sink = test_print;
}

void test_set_wasm(host_t *h)
{
    h->sink = host_print_record;
}

void test_set_counting(host_t *h)
{
    test_kept_t *kept = h->kept;

    h->sink = test_hear_counts;
    h->ud = &kept->listener;
}

void test_set_noting(host_t *h)
{
    test_kept_t *kept = h->kept;

    h->sink = test_hear_noted;
    h->ud = &kept->noted;
}
