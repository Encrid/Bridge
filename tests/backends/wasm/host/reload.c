/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <string.h>

#include "cases.h"

void test_reload_nothing(host_t *h)
{
    bridge_artifact_t a;

    if (host_open(h))
        return;
    test_state_ticker(&a, "game:ticker@0.2.0", "ticker-ten");
    host_reload(h, &a);
    host_close(h);
}

void test_revive(host_t *h)
{
    bridge_artifact_t a;

    if (host_open(h))
        return;
    test_state_ticker(&a, "game:ticker@0.1.0", "ticker-trip");
    host_load(h, &a);
    host_run_frames(h, 4);
    test_say_modules(h);
    test_state_ticker(&a, "game:ticker@0.2.0", "ticker-ten");
    host_reload(h, &a);
    host_run_frames(h, 2);
    test_say_modules(h);
    host_close(h);
}
