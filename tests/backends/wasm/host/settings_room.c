/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "cases.h"

static void load_built(host_t *h, const char *file, const char *config,
    int64_t persistent)
{
    bridge_artifact_t a;

    if (host_open(h))
        return;
    test_state_configured(h, &a, "game:settings@0.1.0");
    a.bytes = host_read_module(file, &a.len);
    a.cfg.config = config;
    a.cfg.persistent = persistent;
    host_load(h, &a);
    test_say_modules(h);
    host_close(h);
}

void test_name_missing_config(host_t *h)
{
    load_built(h, "settings", "game:Nope", 64);
}

void test_state_small_arena(host_t *h)
{
    load_built(h, "settings", "game:Settings", 8);
}

void test_leave_no_room(host_t *h)
{
    h->cfg.budget.columns = 16;
    load_built(h, "settings", "game:Settings", 64);
}

void test_configure_table(host_t *h)
{
    load_built(h, "settings-table", "game:Settings", 64);
}
