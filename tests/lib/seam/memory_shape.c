/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdlib.h>
#include <string.h>

#include "cases.h"

static bridge_setting_t test_shape[4] = {
    { "capabilities", BRIDGE_INT, {}, { .i = HOST_TESTS_KEEPS_ALL } },
    { "reservation", BRIDGE_INT, {}, { .i = HOST_TESTS_RESERVES } },
    { "guard", BRIDGE_INT, {}, { .i = HOST_TESTS_GRANULE } },
    { "granule", BRIDGE_INT, {}, { .i = HOST_TESTS_GRANULE } }
};

void test_state_shape(host_t *h)
{
    const char *equals = strchr(h->arg, '=');
    size_t len = (size_t)(equals - h->arg);
    uint32_t k = 1;

    while (k < 3 && strncmp(test_shape[k].key, h->arg, len))
        k++;
    test_shape[k].v.i = strtoll(equals + 1, nullptr, 0);
    h->backends[0].settings = test_shape;
    h->backends[0].setting_count = 4;
    host_open_only(h);
}
