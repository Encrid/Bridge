/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "cases.h"

static const bridge_setting_t HUGE[4] = {
    { "capabilities", BRIDGE_INT, {}, { .i = HOST_TESTS_KEEPS_ALL } },
    { "reservation", BRIDGE_INT, {}, { .i = INT64_C(1) << 50 } },
    { "guard", BRIDGE_INT, {}, { .i = HOST_TESTS_GRANULE } },
    { "granule", BRIDGE_INT, {}, { .i = HOST_TESTS_GRANULE } }
};

void test_reserve_too_much(host_t *h)
{
    h->backends[0].settings = HUGE;
    h->backends[0].setting_count = 4;
    host_open_only(h);
}
