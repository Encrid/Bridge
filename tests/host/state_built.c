/*
** Copyright (C) 2026 Bridge Owner
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/host/state_built.c -- a module of the tests' backend, loaded.
*/

#include "cases.h"

/*
** test_load_built
**
** Loads a module of the tests' backend that mints `spawns` ids between two
** barriers.
*/
void test_load_built(host_t *h, const char *identity, const char *name,
    int64_t spawns)
{
    bridge_artifact_t a;

    host_state_built(&a, identity, name);
    a.cfg.spawns = spawns;
    host_load(h, &a);
}
