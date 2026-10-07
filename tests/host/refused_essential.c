/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/host/refused_essential.c -- a module the deployment calls
** essential, refused before it exists: its on_error is a key of the config
** that is refused, and does not apply.
**
** failure.md, *When something fails*
*/

#include "cases.h"

/*
** test_refuse_essential
**
** A module that states fail and names a backend the deployment has not is
** refused, and the deck runs on: the next module loads.
*/
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
