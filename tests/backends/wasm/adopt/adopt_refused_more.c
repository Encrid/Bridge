/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
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

int adopt_check_refusals(adopt_t *t, backend_instance_t *one)
{
    return (adopt_check_functions(t, one) || refuse_memory(t));
}
