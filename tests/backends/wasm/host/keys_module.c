/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "cases.h"

static const test_edit_t TEST_MODULE_EDITS[] = {
    { "identity=0", offsetof(test_keys_t, identity), 8, 0 },
    { "reloadable=0", offsetof(test_keys_t, reloadable), 1, BRIDGE_UNSET },
    { "hot_reload=0", offsetof(test_keys_t, hot_reload), 1, BRIDGE_UNSET },
    { "on_error=0", offsetof(test_keys_t, on_error), 1, 0 },
    { "max_system_failures=0", offsetof(test_keys_t, max_system_failures),
        8, 0 },
    { "work_limit=0", offsetof(test_keys_t, work_limit), 8, 0 },
    { "init_timeout=0", offsetof(test_keys_t, init_timeout), 8, 0 },
    { "reload_timeout=0", offsetof(test_keys_t, reload_timeout), 8, 0 },
    { "shutdown_timeout=0", offsetof(test_keys_t, shutdown_timeout), 8, 0 },
    { "scratch=0", offsetof(test_keys_t, scratch), 4, 0 },
    { "log_ring=0", offsetof(test_keys_t, log_ring), 4, 0 },
    { "log_ring=8", offsetof(test_keys_t, log_ring), 4, 8 },
    { "spans=0", offsetof(test_keys_t, spans), 4, 0 },
    { "spawns=0", offsetof(test_keys_t, spawns), 8, 0 },
    { "spawns=5000", offsetof(test_keys_t, spawns), 8, 5000 },
    { "persistent=0", offsetof(test_keys_t, persistent), 8, 0 },
    { "changes=0", offsetof(test_keys_t, changes), 8, 0 },
    { "changes=beyond", offsetof(test_keys_t, changes), 8, 1ll << 33 },
    { "loss_count=0", offsetof(test_keys_t, loss_count), 8, 0 },
    { "loss_count=2", offsetof(test_keys_t, loss_count), 8, 2 }
};

void test_edit_module(host_t *h)
{
    const test_edit_t *e = test_find_edit(TEST_MODULE_EDITS, h->arg);
    char identity[64];
    bridge_artifact_t a;

    if (host_open(h))
        return;
    test_state_small(&a, identity, "ok");
    test_put_key((char *)&a.cfg + e->at, e->size, e->value);
    host_load(h, &a);
    host_close(h);
}

void test_load_twice(host_t *h)
{
    if (host_open(h))
        return;
    test_load_small(h, "ok");
    test_load_small(h, "ok");
    host_close(h);
}

void test_spawn_past_entities(host_t *h)
{
    char identity[64];
    bridge_artifact_t a;

    if (host_open(h))
        return;
    test_state_small(&a, identity, "ok");
    a.cfg.spawns = 3000;
    host_load(h, &a);
    test_state_small(&a, identity, "image");
    a.cfg.spawns = 3000;
    host_load(h, &a);
    host_close(h);
}

void test_edit_phase_names(host_t *h)
{
    bridge_phase_name_t names[2] = { { "tick", "update" },
        { "tick", "update" } };
    char identity[64];
    bridge_artifact_t a;

    if (host_open(h))
        return;
    test_state_small(&a, identity, "ok");
    names[1].from = *h->arg ? names[1].from : nullptr;
    a.cfg.phases = names;
    a.cfg.phase_count = 2;
    host_load(h, &a);
    host_close(h);
}

void test_order_nobody(host_t *h)
{
    bridge_order_t pair = { "game:mover:move", "view:halver:halve" };

    if (*h->arg == 'b')
        pair.before = nullptr;
    if (*h->arg == 'a')
        pair.after = nullptr;
    h->cfg.order = &pair;
    h->cfg.order_count = 1;
    host_open_only(h);
}
