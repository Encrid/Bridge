/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/host/backend_settings.c -- a backend's own settings, handed over
** unread: a backend that says each one it was handed, of each kind.
*/

#include "cases.h"

static const bridge_setting_t TEST_BACKEND_SETTINGS[4] = {
    { .key = "cached", .kind = BRIDGE_BOOL, .v = { .b = true } },
    { .key = "workers", .kind = BRIDGE_INT, .v = { .i = 7 } },
    { .key = "ratio", .kind = BRIDGE_FLOAT, .v = { .f = 1.5 } },
    { .key = "where", .kind = BRIDGE_STRING, .v = { .s = "somewhere" } }
};

/*
** test_hand_settings
**
** A backend handed a setting of each kind: it says what it read of each,
** and does not start.
*/
void test_hand_settings(host_t *h)
{
    h->backends[1].library = NATIVE_REFUSING_PATH;
    h->backends[1].settings = TEST_BACKEND_SETTINGS;
    h->backends[1].setting_count = 4;
    host_open_only(h);
}
