/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <string.h>

#include "cases.h"

static const bridge_setting_t TEST_BACKEND_SETTINGS[4] = {
    { .key = "cached", .kind = BRIDGE_BOOL, .v = { .b = true } },
    { .key = "workers", .kind = BRIDGE_INT, .v = { .i = 7 } },
    { .key = "ratio", .kind = BRIDGE_FLOAT, .v = { .f = 1.5 } },
    { .key = "where", .kind = BRIDGE_STRING, .v = { .s = "somewhere" } }
};

void test_hand_settings(host_t *h)
{
    bridge_setting_t settings[4];

    memcpy(settings, TEST_BACKEND_SETTINGS, sizeof(settings));
    if (!strcmp(h->arg, "bool"))
        settings[0].v.b = false;
    if (!strcmp(h->arg, "int"))
        settings[1].v.i = 8;
    if (!strcmp(h->arg, "float"))
        settings[2].v.f = 2.5;
    if (!strcmp(h->arg, "string"))
        settings[3].v.s = "elsewhere";
    h->backends[1].library = PLAIN_REFUSING_PATH;
    h->backends[1].settings = settings;
    h->backends[1].setting_count = 4;
    host_open_only(h);
}
