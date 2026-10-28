/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <string.h>

#include "cases.h"

static bridge_setting_t test_wide[6] = {
    { .key = "e16", .kind = BRIDGE_INT, .v = { .i = 1 } },
    { .key = "e32", .kind = BRIDGE_INT, .v = { .i = 1 } },
    { .key = "eu8", .kind = BRIDGE_INT, .v = { .i = 1 } },
    { .key = "names", .kind = BRIDGE_STRING, .v = { .s = "a" } },
    { .key = "pair", .kind = BRIDGE_INT, .v = { .i = 1 } },
    { .key = "mask64", .kind = BRIDGE_INT, .v = { .i = 1 } }
};

static uint32_t pick_settings(const host_t *h)
{
    if (!strcmp(h->arg, "low"))
        test_wide[2].v.i = -1;
    if (!strcmp(h->arg, "high"))
        test_wide[2].v.i = 256;
    if (!strcmp(h->arg, "masktop"))
        test_wide[5].v.i = INT64_MIN;
    if (!strcmp(h->arg, "names"))
        return (4);
    if (!strcmp(h->arg, "pair")) {
        test_wide[3] = test_wide[4];
        return (4);
    }
    if (!strncmp(h->arg, "mask", 4)) {
        test_wide[3] = test_wide[5];
        return (4);
    }
    return (3);
}

void test_state_wide(host_t *h)
{
    bridge_artifact_t a;
    uint32_t count = pick_settings(h);

    if (host_open(h))
        return;
    test_state_guarded_minting(&a, "game:settings@0.1.0", "settings-wide");
    a.cfg.config = "game:Settings";
    a.cfg.settings = test_wide;
    a.cfg.setting_count = count;
    a.cfg.persistent = 64;
    host_load(h, &a);
    host_run_frames(h, 1);
    host_close(h);
}
