/*
** Copyright (C) 2026 Bridge Owner
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/support/host_built.c -- a module of the tests' backend, stated as a
** deployment states one.
*/

#include "host.h"

/*
** host_state_built
**
** States a module that is compiled into the tests' backend, by its name.
*/
void host_state_built(bridge_artifact_t *a, const char *identity,
    const char *name)
{
    host_state_native(a, identity, name);
    a->backend = "tests";
}
