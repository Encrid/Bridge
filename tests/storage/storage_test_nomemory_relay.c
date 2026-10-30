/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <string.h>

#include "storage_test.h"

enum storage_test_end storage_test_relay_live(storage_test_t *t)
{
    bridge_artifact_t a = storage_test_state(LAYOUT0_PATH, 8, 1 << 20, 4096);
    bool empty = !strcmp(t->what, "nomemoryrelayempty");

    t->w = bridge_create(&t->cfg, storage_test_hear_memory, t);
    if (!t->w)
        return (STORAGE_TEST_DONE);
    a.cfg.reloadable = BRIDGE_TRUE;
    a.cfg.hot_reload = BRIDGE_TRUE;
    if (!bridge_load(t->w, &a) && (empty || !bridge_run_phase(t->w, "spawn"))) {
        a.path = empty ? LAYOUT15_PATH : LAYOUT1_PATH;
        (void)bridge_reload(t->w, &a, 1);
    }
    return (STORAGE_TEST_DONE);
}
