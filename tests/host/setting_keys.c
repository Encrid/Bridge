/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <string.h>

#include "cases.h"

static void hand_wrong(host_t *h, bridge_setting_t *s)
{
    if (!strcmp(h->arg, "twice")) {
        s[4].v.s = "abcdef";
        s[9] = (bridge_setting_t){ .key = "title", .kind = BRIDGE_STRING,
            .v = { .s = "xy" } };
    }
    if (!strcmp(h->arg, "nulltext"))
        s[5].v.s = nullptr;
    if (!strcmp(h->arg, "nullkey"))
        s[9].key = nullptr;
    if (!strcmp(h->arg, "nokind"))
        s[0].kind = (enum bridge_setting_kind)5;
    if (!strcmp(h->arg, "negative"))
        s[7].v.i = -1;
}

void test_hand_keys(host_t *h)
{
    bridge_artifact_t a;

    hand_wrong(h, test_get_noted(h)->settings);
    if (host_open(h))
        return;
    test_state_configured(h, &a, "game:settings@0.1.0");
    host_load(h, &a);
    host_run_frames(h, 1);
    host_close(h);
}
