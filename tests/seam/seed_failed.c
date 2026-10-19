/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <string.h>

#include "cases.h"

static const bridge_setting_t SEED[1] = {
    { .key = "x", .kind = BRIDGE_INT, .v = { .i = 99 } }
};

static void load_six(host_t *h, const char *identity, const char *file)
{
    bridge_artifact_t a;

    host_state_plain(&a, identity, nullptr);
    host_name_module(&a, file);
    if (!strcmp(file, "six-seeded")) {
        a.cfg.config = "six:Settings";
        a.cfg.settings = SEED;
        a.cfg.setting_count = 1;
    }
    host_load(h, &a);
}

void test_seed_failed_load(host_t *h)
{
    if (host_open(h))
        return;
    load_six(h, "six:holder@0.1.0", "six");
    host_run_frames(h, 1);
    load_six(h, "six:seeded@0.1.0", "six-seeded");
    host_run_frames(h, 1);
    host_close(h);
}
