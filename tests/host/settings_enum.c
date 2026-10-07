/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge-ASM700.
**
** All rights reserved.
**
** tests/host/settings_enum.c -- a setting of an enum field, and a config
** that names no component, or none there is.
**
** config.md, *A module's own settings are a component*
*/

#include "cases.h"

/*
** load_named
**
** Loads the settings module with its config naming `config`, and a share of
** the arena of `persistent` bytes.
*/
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

/*
** test_overflow_enum
**
** The enum is an int8_t, as its author stated: a number past it is
** refused.
*/
void test_overflow_enum(host_t *h)
{
    bridge_setting_t *mode = &test_get_noted(h)->settings[3];

    mode->kind = BRIDGE_INT;
    mode->v.i = 200;
    load_named(h, "game:Settings", 64);
}

/*
** test_misname_constant
**
** A name the enum has no constant for.
*/
void test_misname_constant(host_t *h)
{
    test_get_noted(h)->settings[3].v.s = "Sometimes";
    load_named(h, "game:Settings", 64);
}

/*
** test_state_no_config
**
** Settings handed to a module that states it has no config.
*/
void test_state_no_config(host_t *h)
{
    load_named(h, BRIDGE_NO_CONFIG, 64);
}

/*
** test_leave_config_unset
**
** A config nobody stated.
*/
void test_leave_config_unset(host_t *h)
{
    load_named(h, nullptr, 64);
}
