/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>

#include "region.h"
#include "storage_test.h"

enum storage_test_end storage_test_region_full([[maybe_unused]] storage_test_t *t)
{
    range_t gaps[2] = {{ 10, 5 }, { 30, 5 }};
    region_t r = { .gaps = gaps, .count = 2, .room = 2, .used = 100 };

    region_give(&r, 20, 5);
    printf("region full: count %u\n", r.count);
    return (STORAGE_TEST_LEFT);
}
