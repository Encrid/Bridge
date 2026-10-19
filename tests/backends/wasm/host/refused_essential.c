/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "cases.h"

void test_refuse_essential(host_t *h)
{
    char identity[64];
    bridge_artifact_t a;

    if (host_open(h))
        return;
    test_state_small(&a, identity, "ok");
    a.cfg.on_error = BRIDGE_FAIL;
    a.backend = "native";
    host_load(h, &a);
    test_load_small(h, "image");
    host_close(h);
}
