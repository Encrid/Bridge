/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>

#include "cases.h"

static const bridge_setting_t TEST_NO_RELOAD[4] = {
    { "capabilities", BRIDGE_INT, {}, { .i = 1 << 6 } },
    { "reservation", BRIDGE_INT, {}, { .i = 0 } },
    { "guard", BRIDGE_INT, {}, { .i = 0 } },
    { "granule", BRIDGE_INT, {}, { .i = 0 } }
};

void test_call_with_nulls(host_t *h)
{
    bridge_artifact_t a;

    printf("bridge_create with no config: %s\n",
        bridge_create(nullptr, h->sink, h->ud) ? "made" : "refused");
    printf("bridge_destroy of no world: %d\n", bridge_destroy(nullptr));
    if (host_open(h))
        return;
    test_load_guarded(h, "game:mover@0.1.0", "mover");
    printf("bridge_run_phase with no name: %d\n",
        bridge_run_phase(h->b, nullptr));
    printf("bridge_list_modules with no list: %d\n",
        bridge_list_modules(h->b, nullptr, 0));
    printf("bridge_unload with no identity: %d\n",
        bridge_unload(h->b, nullptr));
    test_state_guarded(&a, "game:loose@0.1.0", "mover");
    a.backend = nullptr;
    host_load(h, &a);
    host_close(h);
}

void test_hot_reload_unkept(host_t *h)
{
    bridge_artifact_t a;

    h->backends[1].settings = TEST_NO_RELOAD;
    h->backends[1].setting_count = 4;
    if (host_open(h))
        return;
    host_state_plain(&a, "test:ok@0.1.0", nullptr);
    host_name_module(&a, "ok");
    a.cfg.reloadable = BRIDGE_TRUE;
    a.cfg.hot_reload = BRIDGE_TRUE;
    host_load(h, &a);
    host_close(h);
}

void test_repair_fails_again(host_t *h)
{
    bridge_artifact_t a;

    if (host_open(h))
        return;
    test_state_guarded_minting(&a, "game:thing@0.1.0", "badobs");
    a.cfg.on_error = BRIDGE_FAIL;
    host_load(h, &a);
    host_unload(h, "game:thing");
    test_state_guarded_minting(&a, "game:thing@0.2.0", "badobs");
    a.cfg.on_error = BRIDGE_FAIL;
    host_load(h, &a);
    test_say_modules(h);
    host_close(h);
}

void test_reload_into_conflict(host_t *h)
{
    bridge_artifact_t a;

    if (host_open(h))
        return;
    test_load_guarded(h, "game:mover@0.1.0", "mover");
    test_state_guarded(&a, "game:thing@0.1.0", "duo");
    a.cfg.reloadable = BRIDGE_TRUE;
    a.cfg.hot_reload = BRIDGE_TRUE;
    host_load(h, &a);
    test_state_guarded(&a, "game:thing@0.2.0", "writer");
    a.cfg.reloadable = BRIDGE_TRUE;
    a.cfg.hot_reload = BRIDGE_TRUE;
    host_reload(h, &a);
    test_say_modules(h);
    host_close(h);
}

void test_reload_colliding(host_t *h)
{
    bridge_artifact_t a;
    char moved[32];

    if (host_open(h))
        return;
    test_state_ticker(&a, "st:collide@0.1.0", h->arg);
    host_load(h, &a);
    host_run_frames(h, 1);
    snprintf(moved, sizeof(moved), "%s-moved", h->arg);
    test_state_ticker(&a, "st:collide@0.2.0", moved);
    host_reload(h, &a);
    test_say_modules(h);
    host_close(h);
}

void test_reload_then_stop(host_t *h)
{
    bridge_artifact_t a;

    if (host_open(h))
        return;
    test_state_guarded(&a, "view:sour@0.1.0", "sour");
    a.cfg.read = TEST_GAME;
    a.cfg.read_count = 2;
    a.cfg.max_system_failures = 1;
    a.cfg.on_error = BRIDGE_FAIL;
    host_load(h, &a);
    test_state_guarded(&a, "game:marker@0.1.0", "marker");
    a.cfg.reloadable = BRIDGE_TRUE;
    a.cfg.hot_reload = BRIDGE_TRUE;
    a.cfg.spawns = 4;
    host_load(h, &a);
    a.cfg.identity = "game:marker@0.2.0";
    host_reload(h, &a);
    test_say_modules(h);
    host_close(h);
}
