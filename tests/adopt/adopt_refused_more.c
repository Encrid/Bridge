/*
** Copyright (C) 2026 Bridge Owner
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/adopt/adopt_refused_more.c -- what a backend refuses of whoever
** calls it: a memory handed to the wasm backend twice under one name, and
** an entry the native backend's SDK does not have.
*/

#include "adopt.h"

static int refuse_memory(adopt_t *t)
{
    backend_err_t err = {};
    backend_memory_import_t imports[2] = {{ "bridge", "shared", t->memory },
        { "bridge", "shared", t->memory }};
    backend_instance_t *i = t->api.instance_create(t->module, imports, 2,
        &err);

    if (i)
        t->api.instance_destroy(i);
    return (adopt_say_refused("a memory given twice under one name", !i,
        &err));
}

static int refuse_entry(const backend_api_t *api, backend_module_t *m)
{
    backend_err_t err = {};
    backend_entry_t entry = { "bridge", "teleport", nullptr, nullptr,
        nullptr };
    backend_instance_t *i = api->instance_create(m, nullptr, 0, &err);
    int refused = api->define_func(i, &entry, &err);

    api->instance_destroy(i);
    return (adopt_say_refused("an entry the native SDK has not", refused,
        &err));
}

/*
** adopt_check_native_entry
**
** The native backend, asked to define an entry that is none of the ABI's.
*/
int adopt_check_native_entry(void)
{
    backend_api_t api;
    backend_err_t err = {};
    backend_source_t source = { .path = NMOVER_PATH };
    backend_t *be;
    backend_module_t *m;

    if (!backend_open(NATIVE_PATH, &api, &err))
        return (adopt_fail("%s", err.text));
    be = api.backend_create(nullptr, 0, &err);
    m = api.module_compile(be, &source, &err);
    if (!m || refuse_entry(&api, m))
        return (adopt_fail("the native backend: %s", err.text));
    api.module_destroy(m);
    api.backend_destroy(be);
    return (0);
}

/*
** adopt_check_refusals
**
** Each thing a backend refuses of whoever calls it is refused, and says
** why.
*/
int adopt_check_refusals(adopt_t *t, backend_instance_t *one)
{
    return (adopt_check_functions(t, one) || refuse_memory(t)
        || adopt_check_native_entry());
}
