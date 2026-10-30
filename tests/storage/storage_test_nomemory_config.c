/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "storage_test.h"

static const bridge_setting_t SETTINGS[10] = {
    { .key = "vsync", .kind = BRIDGE_BOOL, .v = { .b = true } },
    { .key = "width", .kind = BRIDGE_INT, .v = { .i = 1280 } },
    { .key = "scale", .kind = BRIDGE_FLOAT, .v = { .f = 1.5 } },
    { .key = "mode", .kind = BRIDGE_STRING, .v = { .s = "ADAPTIVE" } },
    { .key = "title", .kind = BRIDGE_STRING, .v = { .s = "hello" } },
    { .key = "path", .kind = BRIDGE_STRING, .v = { .s = "some/long/path.cfg" } },
    { .key = "small", .kind = BRIDGE_INT, .v = { .i = 7 } },
    { .key = "flags", .kind = BRIDGE_INT, .v = { .i = 3 } },
    { .key = "ratio", .kind = BRIDGE_INT, .v = { .i = 3 } },
    { .key = "big", .kind = BRIDGE_INT, .v = { .i = 8'589'934'592ll } }
};

static const char *const UPDATE[1] = { "update" };

static const char *const READ[2] = { "view", "menu" };

static const bridge_order_t ORDER[1] = {{ "game:first:tick",
    "game:second:tock" }};

static void load_other(bridge_world_t *w, bridge_artifact_t a)
{
    a.cfg.identity = "game:other@0.1.0";
    a.cfg.read = READ;
    a.cfg.read_count = 2;
    (void)bridge_load(w, &a);
}

enum storage_test_end storage_test_configure_live(storage_test_t *t)
{
    bridge_artifact_t a = storage_test_state(BUILT_DIR "/settings" BUILT_SUFFIX, 6, 1 << 20, 1 << 20);

    t->cfg.phases = UPDATE;
    t->cfg.phase_count = 1;
    t->cfg.order = ORDER;
    t->cfg.order_count = 1;
    t->w = bridge_create(&t->cfg, storage_test_hear_memory, t);
    if (!t->w)
        return (STORAGE_TEST_DONE);
    load_other(t->w, a);
    a.cfg.identity = "game:settings@0.1.0";
    a.cfg.config = "game:Settings";
    a.cfg.settings = SETTINGS;
    a.cfg.setting_count = 10;
    a.cfg.persistent = 128;
    (void)bridge_load(t->w, &a);
    (void)bridge_unload(t->w, a.cfg.identity);
    return (STORAGE_TEST_DONE);
}
