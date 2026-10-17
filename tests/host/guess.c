/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <string.h>

#include "cases.h"

static void load_named(host_t *h, const char *identity, const char *file,
    int64_t spawns)
{
    bridge_artifact_t a;

    host_state_plain(&a, identity, nullptr);
    host_name_module(&a, file);
    a.cfg.spawns = spawns;
    a.cfg.changes = 1024;
    host_load(h, &a);
}

static void run_beside_hold(host_t *h, const char *identity)
{
    if (host_open(h))
        return;
    load_named(h, "test:hold@0.1.0", "hold", 4);
    load_named(h, identity, h->arg, BRIDGE_NONE);
    host_run_frames(h, 2);
    host_close(h);
}

void test_guess(host_t *h)
{
    run_beside_hold(h, "test:guess@0.1.0");
}

void test_stick(host_t *h)
{
    run_beside_hold(h, "test:stick@0.1.0");
}

void test_guess_later(host_t *h)
{
    if (host_open(h))
        return;
    load_named(h, "test:hold@0.1.0", "hold", 4);
    if (strstr(h->arg, "apart"))
        test_load_guarded(h, "game:hearer@0.1.0", "hearer");
    if (strstr(h->arg, "carried"))
        load_named(h, "test:stick@0.1.0", "stick-given", BRIDGE_NONE);
    host_run_frames(h, 1);
    load_named(h, "test:guess@0.1.0", "guess-given", BRIDGE_NONE);
    host_run_frames(h, 2);
    test_say_modules(h);
    host_close(h);
}
