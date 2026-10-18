/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "native_inside.h"

BACKEND_EXPORT int define_func(backend_instance_t *i,
    const backend_entry_t *entry, backend_err_t *err)
{
    uint32_t k = 0;

    while (k < NATIVE_ENTRIES && strcmp(entry->field, NATIVE_ENTRY_NAMES[k]))
        k++;
    if (k == NATIVE_ENTRIES || strcmp(entry->ns, BRIDGE_NAMESPACE)) {
        snprintf(err->text, sizeof(err->text), "the native SDK has no entry "
            "'%.100s'", entry->field);
        return (-1);
    }
    i->fns[k] = entry->fn;
    i->uds[k] = entry->ud;
    return (0);
}

BACKEND_EXPORT backend_func_t *instance_get_func(backend_instance_t *i,
    const char *name, backend_err_t *err)
{
    char symbol[160];
    backend_func_t *f;
    void *fn;

    snprintf(symbol, sizeof(symbol), "bridge_%.150s", name);
    fn = os_lib_get_symbol(i->m->lib, symbol);
    if (!fn) {
        snprintf(err->text, sizeof(err->text), "no exported function '%s'",
            symbol);
        return (nullptr);
    }
    f = calloc(1, sizeof(*f));
    f->instance = i;
    f->fn = (int (*)(const bridge_api_t *))fn;
    f->next = i->funcs;
    i->funcs = f;
    return (f);
}

BACKEND_EXPORT int call(backend_func_t *f, int32_t *status,
    [[maybe_unused]] backend_err_t *err)
{
    backend_instance_t *i = f->instance;
    uint64_t before = i->calling;

    i->calling = os_get_thread_id();
    i->refused = 0;
    *status = f->fn(&i->api);
    i->calling = before;
    return (0);
}
