/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "cases.h"

static const bridge_setting_t TEST_SETTING[1] = {
    { "cache", BRIDGE_STRING, {}, { .s = "somewhere" } }
};

void test_hand_setting(host_t *h)
{
    h->backends[0].settings = TEST_SETTING;
    h->backends[0].setting_count = 1;
    host_open_only(h);
}
