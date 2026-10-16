/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "cases.h"

static void load_named(host_t *h, const char *config, int64_t persistent)
{
    bridge_artifact_t a;

    if (host_open(h))
        return;
    test_state_configured(h, &a, "game:settings@0.1.0");
    a.cfg.config = config;
    a.cfg.persistent = persistent;
    host_load(h, &a);
    test_say_modules(h);
    host_close(h);
}

void test_overflow_enum(host_t *h)
{
    bridge_setting_t *mode = &test_get_noted(h)->settings[3];

    mode->kind = BRIDGE_INT;
    mode->v.i = 200;
    load_named(h, "game:Settings", 64);
}

void test_misname_constant(host_t *h)
{
    test_get_noted(h)->settings[3].v.s = "Sometimes";
    load_named(h, "game:Settings", 64);
}

void test_state_no_config(host_t *h)
{
    load_named(h, BRIDGE_NO_CONFIG, 64);
}

void test_leave_config_unset(host_t *h)
{
    load_named(h, nullptr, 64);
}
