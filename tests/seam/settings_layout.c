/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "cases.h"

static const bridge_setting_t WIDTHS[2] = {
    { .key = "width", .kind = BRIDGE_INT, .v = { .i = 1280 } },
    { .key = "width", .kind = BRIDGE_INT, .v = { .i = 7 } }
};

static void state_nine(bridge_artifact_t *a, const char *identity,
    const char *file, const bridge_setting_t *width)
{
    host_state_guarded(a, identity, file);
    a->cfg.reloadable = BRIDGE_TRUE;
    a->cfg.hot_reload = BRIDGE_TRUE;
    a->cfg.config = "nine:Settings";
    a->cfg.settings = width;
    a->cfg.setting_count = 1;
}

static void load_nine(host_t *h, const char *identity, const char *file,
    const bridge_setting_t *width)
{
    bridge_artifact_t a;

    state_nine(&a, identity, file, width);
    host_load(h, &a);
    host_run_frames(h, 1);
}

void test_reload_settings_layout(host_t *h)
{
    bridge_artifact_t a;

    if (host_open(h))
        return;
    load_nine(h, "nine:first@0.1.0", "nine", WIDTHS);
    load_nine(h, "nine:second@0.1.0", "nine", WIDTHS + 1);
    state_nine(&a, "nine:first@0.2.0", "nine-wide", WIDTHS);
    host_reload(h, &a);
    host_run_frames(h, 1);
    load_nine(h, "nine:second@0.1.0", "nine-wide", WIDTHS + 1);
    host_close(h);
}
