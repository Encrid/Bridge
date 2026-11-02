/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdlib.h>

#include "cases.h"

void test_reserve_size(host_t *h)
{
    int64_t size = INT64_C(1) << strtol(h->arg, nullptr, 10);
    const bridge_setting_t settings[4] = {
        { "capabilities", BRIDGE_INT, {}, { .i = HOST_TESTS_KEEPS_ALL } },
        { "reservation", BRIDGE_INT, {}, { .i = size } },
        { "guard", BRIDGE_INT, {}, { .i = size } },
        { "granule", BRIDGE_INT, {}, { .i = HOST_TESTS_GRANULE } }
    };

    h->backends[0].settings = settings;
    h->backends[0].setting_count = 4;
    host_open_only(h);
}
