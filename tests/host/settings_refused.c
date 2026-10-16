/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
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

void test_misname_setting(host_t *h)
{
    test_get_noted(h)->settings[0].key = "vsynk";
    load_settings(h);
}

void test_mistype_setting(host_t *h)
{
    bridge_setting_t *scale = &test_get_noted(h)->settings[2];

    scale->kind = BRIDGE_STRING;
    scale->v.s = "fast";
    load_settings(h);
}

void test_overflow_text(host_t *h)
{
    test_get_noted(h)->settings[4].v.s = "twenty characters...";
    load_settings(h);
}

void test_overflow_integer(host_t *h)
{
    test_get_noted(h)->settings[6].v.i = 300;
    load_settings(h);
}
