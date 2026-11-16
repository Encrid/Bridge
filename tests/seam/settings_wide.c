/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <string.h>

#include "cases.h"

static const bridge_setting_t TEST_WIDE[6] = {
    { .key = "e16", .kind = BRIDGE_INT, .v = { .i = 1 } },
    { .key = "e32", .kind = BRIDGE_INT, .v = { .i = 1 } },
    { .key = "eu8", .kind = BRIDGE_INT, .v = { .i = 1 } },
    { .key = "names", .kind = BRIDGE_STRING, .v = { .s = "a" } },
    { .key = "pair", .kind = BRIDGE_INT, .v = { .i = 1 } },
    { .key = "mask64", .kind = BRIDGE_INT, .v = { .i = 1 } }
};

static uint32_t pick_settings(const host_t *h, bridge_setting_t *wide)
{
    if (!strcmp(h->arg, "low"))
        wide[2].v.i = -1;
    if (!strcmp(h->arg, "high"))
        wide[2].v.i = 256;
    if (!strcmp(h->arg, "masktop"))
        wide[5].v.i = INT64_MIN;
    if (!strcmp(h->arg, "names"))
        return (4);
    if (!strcmp(h->arg, "pair")) {
        wide[3] = wide[4];
        return (4);
    }
    if (!strncmp(h->arg, "mask", 4)) {
        wide[3] = wide[5];
        return (4);
    }
    return (3);
}

void test_state_wide(host_t *h)
{
    bridge_artifact_t a;
    bridge_setting_t wide[6];
    uint32_t count = 0;

    memcpy(wide, TEST_WIDE, sizeof(wide));
    count = pick_settings(h, wide);
    if (host_open(h))
        return;
    test_state_guarded_minting(&a, "game:settings@0.1.0", "settings-wide");
    a.cfg.config = "game:Settings";
    a.cfg.settings = wide;
    a.cfg.setting_count = count;
    a.cfg.persistent = 64;
    host_load(h, &a);
    host_run_frames(h, 1);
    host_close(h);
}
