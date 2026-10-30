/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "storage_test.h"

static void use_module(bridge_world_t *w, bridge_artifact_t a)
{
    (void)bridge_run_phase(w, "spawn");
    (void)bridge_run_phase(w, "attach_a");
    (void)bridge_run_phase(w, "attach_b_half");
    a.cfg.spawns = 2;
    a.cfg.persistent = 64;
    (void)bridge_reload(w, &a, 1);
    (void)bridge_run_phase(w, "look");
    (void)bridge_unload(w, a.cfg.identity);
}

enum storage_test_end storage_test_live(storage_test_t *t)
{
    bridge_artifact_t a = storage_test_state(STORE_PATH, 6, 1 << 20, 1 << 20);

    t->w = bridge_create(&t->cfg, storage_test_hear_memory, t);
    if (!t->w)
        return (STORAGE_TEST_DONE);
    a.cfg.reloadable = BRIDGE_TRUE;
    a.cfg.hot_reload = BRIDGE_TRUE;
    if (!bridge_load(t->w, &a))
        use_module(t->w, a);
    return (STORAGE_TEST_DONE);
}
