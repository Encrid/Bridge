/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "cases.h"

void test_load_built(host_t *h, const char *identity, const char *name,
    int64_t spawns)
{
    bridge_artifact_t a;

    host_state_built(&a, identity, name);
    a.cfg.spawns = spawns;
    host_load(h, &a);
}

void test_state_built_minting(bridge_artifact_t *a, const char *identity,
    const char *name)
{
    host_state_built(a, identity, name);
    a->cfg.spawns = 32;
}
