/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "cases.h"

constexpr int64_t BEYOND = 0x1'0000'0008;

static void reload_beyond(host_t *h, const char *identity, const char *file)
{
    bridge_artifact_t a;

    test_state_ticker(&a, identity, file);
    a.cfg.persistent = BEYOND;
    host_reload(h, &a);
}

void test_reload_beyond_reach(host_t *h)
{
    bridge_artifact_t a;

    if (host_open(h))
        return;
    test_state_ticker(&a, "game:ticker@0.1.0", "ticker");
    host_load(h, &a);
    host_run_frames(h, 1);
    reload_beyond(h, "game:ticker@0.2.0", "ticker-ten");
    reload_beyond(h, "game:ticker@0.3.0", "ticker-alloc");
    test_say_modules(h);
    host_close(h);
}
