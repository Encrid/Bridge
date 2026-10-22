/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <string.h>

#include "cases.h"

typedef struct {
    const char *arg;
    uint32_t at;
    bridge_setting_t s;
} test_setting_edit_t;

static const test_setting_edit_t TEST_EDITS[] = {
    { "modeint", 3, { .key = "mode", .kind = BRIDGE_INT, .v = { .i = 1 } } },
    { "modelow", 3, { .key = "mode", .kind = BRIDGE_INT, .v = { .i = -200 } } },
    { "modebool", 3, { .key = "mode", .kind = BRIDGE_BOOL,
        .v = { .b = true } } },
    { "flagsbool", 7, { .key = "flags", .kind = BRIDGE_BOOL,
        .v = { .b = true } } },
    { "flagshigh", 7, { .key = "flags", .kind = BRIDGE_INT,
        .v = { .i = 4'294'967'296ll } } },
    { "kindzero", 0, { .key = "vsync", .v = { .b = true } } },
    { "titel", 4, { .key = "titel", .kind = BRIDGE_STRING,
        .v = { .s = "hello" } } },
    { "vsyncint", 0, { .key = "vsync", .kind = BRIDGE_INT, .v = { .i = 1 } } },
    { "vsyncfalse", 0, { .key = "vsync", .kind = BRIDGE_BOOL,
        .v = { .b = false } } },
    { "smallnegative", 6, { .key = "small", .kind = BRIDGE_INT,
        .v = { .i = -1 } } },
    { "widthfloat", 1, { .key = "width", .kind = BRIDGE_FLOAT,
        .v = { .f = 1.5 } } },
    { "pathint", 5, { .key = "path", .kind = BRIDGE_INT, .v = { .i = 5 } } },
    { "titleint", 4, { .key = "title", .kind = BRIDGE_INT, .v = { .i = 5 } } }
};

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

static void hand_edit(host_t *h, bridge_setting_t *s)
{
    constexpr size_t count = sizeof(TEST_EDITS) / sizeof(*TEST_EDITS);

    for (size_t i = 0; i < count; i++)
        if (!strcmp(h->arg, TEST_EDITS[i].arg))
            s[TEST_EDITS[i].at] = TEST_EDITS[i].s;
}

static void hand_config(const host_t *h, bridge_artifact_t *a)
{
    if (!strcmp(h->arg, "nopersistent"))
        a->cfg.persistent = BRIDGE_NONE;
    if (!strcmp(h->arg, "configenum"))
        a->cfg.config = "game:mode_t";
}

void test_hand_keys(host_t *h)
{
    bridge_artifact_t a;

    hand_wrong(h, test_get_noted(h)->settings);
    hand_edit(h, test_get_noted(h)->settings);
    if (host_open(h))
        return;
    test_state_configured(h, &a, "game:settings@0.1.0");
    hand_config(h, &a);
    host_load(h, &a);
    host_run_frames(h, 1);
    host_close(h);
}
