/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdlib.h>
#include <string.h>

#include "cases.h"

static const test_edit_t TEST_MODULE_EDITS[] = {
    { "identity=0", offsetof(test_keys_t, identity), 8, 0 },
    { "reloadable=0", offsetof(test_keys_t, reloadable), 1, BRIDGE_UNSET },
    { "hot_reload=0", offsetof(test_keys_t, hot_reload), 1, BRIDGE_UNSET },
    { "on_error=0", offsetof(test_keys_t, on_error), 1, 0 },
    { "max_system_failures=0", offsetof(test_keys_t, max_system_failures),
        8, 0 },
    { "work_limit=0", offsetof(test_keys_t, work_limit), 8, 0 },
    { "element_limit=0", offsetof(test_keys_t, element_limit), 8, 0 },
    { "element_limit=-5", offsetof(test_keys_t, element_limit), 8, -5 },
    { "init_timeout=0", offsetof(test_keys_t, init_timeout), 8, 0 },
    { "reload_timeout=0", offsetof(test_keys_t, reload_timeout), 8, 0 },
    { "shutdown_timeout=0", offsetof(test_keys_t, shutdown_timeout), 8, 0 },
    { "scratch=0", offsetof(test_keys_t, scratch), 4, 0 },
    { "log_ring=0", offsetof(test_keys_t, log_ring), 4, 0 },
    { "log_ring=8", offsetof(test_keys_t, log_ring), 4, 8 },
    { "spans=0", offsetof(test_keys_t, spans), 4, 0 },
    { "spawns=0", offsetof(test_keys_t, spawns), 8, 0 },
    { "spawns=5000", offsetof(test_keys_t, spawns), 8, 5000 },
    { "memory_limit=0", offsetof(test_keys_t, memory_limit), 8, 0 },
    { "memory_limit=-7", offsetof(test_keys_t, memory_limit), 8, -7 },
    { "memory_limit=2097152", offsetof(test_keys_t, memory_limit), 8, 1 << 21 },
    { "log_level=0", offsetof(test_keys_t, log_level), 1, 0 },
    { "log_level=4", offsetof(test_keys_t, log_level), 1, BRIDGE_LOG_WARN },
    { "persistent=0", offsetof(test_keys_t, persistent), 8, 0 },
    { "changes=0", offsetof(test_keys_t, changes), 8, 0 },
    { "changes=beyond", offsetof(test_keys_t, changes), 8, 1ll << 33 },
    { "loss_count=0", offsetof(test_keys_t, loss_count), 8, 0 },
    { "loss_count=2", offsetof(test_keys_t, loss_count), 8, 2 },
    { "max_system_failures=-5", offsetof(test_keys_t, max_system_failures), 8, -5 },
    { "init_timeout=-5", offsetof(test_keys_t, init_timeout), 8, -5 },
    { "reload_timeout=-5", offsetof(test_keys_t, reload_timeout), 8, -5 },
    { "shutdown_timeout=-5", offsetof(test_keys_t, shutdown_timeout), 8, -5 },
    { "persistent=-5", offsetof(test_keys_t, persistent), 8, -5 },
    { "changes=-5", offsetof(test_keys_t, changes), 8, -5 },
    { "loss_count=-5", offsetof(test_keys_t, loss_count), 8, -5 },
    { "scratch=0xFFFFFFF0", offsetof(test_keys_t, scratch), 4, 0xFFFFFFF0 },
    { "scratch=0xFFFFFF7C", offsetof(test_keys_t, scratch), 4, 0xFFFFFF7C },
    { "spans=0xFFFFFFFF", offsetof(test_keys_t, spans), 4, 0xFFFFFFFF },
    { "init_timeout=nolimit", offsetof(test_keys_t, init_timeout), 8, BRIDGE_NO_LIMIT }
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
    test_load_small(h, h->arg);
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
    a.cfg.spawns = strtol(h->arg, nullptr, 10);
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
    names[1].from = *h->arg ? h->arg : nullptr;
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

static void edit_phases(const host_t *h, bridge_phase_name_t *names,
    bridge_artifact_t *a)
{
    names[0] = (bridge_phase_name_t){ "tick", "update" };
    names[1] = (bridge_phase_name_t){ "tock", "update" };
    a->cfg.phases = names;
    a->cfg.phase_count = 2;
    if (!strcmp(h->arg, "phase=from-empty"))
        names[0].from = "";
    if (!strcmp(h->arg, "phase=to-null"))
        names[0].to = nullptr;
    if (!strcmp(h->arg, "phase=to-empty"))
        names[0].to = "";
    if (!strcmp(h->arg, "phase=one-to-one"))
        a->cfg.phase_count = 1;
}

static void edit_more(const host_t *h, bridge_artifact_t *a,
    bridge_phase_name_t *names)
{
    if (!strncmp(h->arg, "identity=", 9))
        a->cfg.identity = h->arg + 9;
    if (!strncmp(h->arg, "config=", 7))
        a->cfg.config = h->arg + 7;
    if (!strncmp(h->arg, "log_ring=", 9)) {
        a->cfg.scratch = 1024;
        a->cfg.log_ring = (uint32_t)strtoul(h->arg + 9, nullptr, 10);
    }
    if (!strcmp(h->arg, "timeouts=nolimit")) {
        a->cfg.init_timeout = BRIDGE_NO_LIMIT;
        a->cfg.reload_timeout = BRIDGE_NO_LIMIT;
    }
    if (!strncmp(h->arg, "phase=", 6))
        edit_phases(h, names, a);
}

void test_edit_module_more(host_t *h)
{
    bridge_phase_name_t names[2];
    char identity[64];
    bridge_artifact_t a;

    if (host_open(h))
        return;
    test_state_small(&a, identity, "ok");
    if (!strncmp(h->arg, "reload=", 7)) {
        a.cfg.reloadable = BRIDGE_TRUE;
        a.cfg.hot_reload = BRIDGE_TRUE;
        host_load(h, &a);
        a.cfg.log_ring = (uint32_t)strtoul(h->arg + 7, nullptr, 10);
        host_reload(h, &a);
        host_close(h);
        return;
    }
    edit_more(h, &a, names);
    host_load(h, &a);
    host_close(h);
}
