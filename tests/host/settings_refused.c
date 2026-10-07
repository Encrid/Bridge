/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge-ASM700.
**
** All rights reserved.
**
** tests/host/settings_refused.c -- a setting that does not fit the field
** it names: each is refused, and says which.
**
** config.md, *A module's own settings are a component*
*/

#include "cases.h"

static void load_settings(host_t *h)
{
    bridge_artifact_t a;

    if (host_open(h))
        return;
    test_state_configured(h, &a, "game:settings@0.1.0");
    host_load(h, &a);
    test_say_modules(h);
    host_close(h);
}

/*
** test_misname_setting
**
** A key the component has no field of.
*/
void test_misname_setting(host_t *h)
{
    test_get_noted(h)->settings[0].key = "vsynk";
    load_settings(h);
}

/*
** test_mistype_setting
**
** Text handed to a float field.
*/
void test_mistype_setting(host_t *h)
{
    bridge_setting_t *scale = &test_get_noted(h)->settings[2];

    scale->kind = BRIDGE_STRING;
    scale->v.s = "fast";
    load_settings(h);
}

/*
** test_overflow_text
**
** Text longer than the field's array.
*/
void test_overflow_text(host_t *h)
{
    test_get_noted(h)->settings[4].v.s = "twenty characters...";
    load_settings(h);
}

/*
** test_overflow_integer
**
** A number the field's integer does not hold.
*/
void test_overflow_integer(host_t *h)
{
    test_get_noted(h)->settings[6].v.i = 300;
    load_settings(h);
}
