/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tests_inside.h"

BACKEND_EXPORT backend_func_t *instance_get_func(backend_instance_t *i,
    const char *name, backend_err_t *err)
{
    char symbol[160];
    backend_func_t *f;
    void *fn;

    snprintf(symbol, sizeof(symbol), "%s%.150s", i->m->keep ? "" : "module_",
        name);
    fn = os_lib_get_symbol(i->m->lib, symbol);
    if (!fn) {
        snprintf(err->text, sizeof(err->text), "no exported function '%s'",
            symbol);
        return (nullptr);
    }
    f = calloc(1, sizeof(*f));
    f->instance = i;
    f->fn = fn;
    f->next = i->funcs;
    i->funcs = f;
    return (f);
}

static int32_t run(const backend_func_t *f)
{
    backend_instance_t *i = f->instance;

    if (!i->m->keep)
        return (((int (*)(const test_api_t *))f->fn)(&i->api));
    i->m->keep(&i->api);
    return (((int (*)(void))f->fn)());
}

BACKEND_EXPORT int call(backend_func_t *f, int32_t *status, backend_err_t *err)
{
    backend_instance_t *i = f->instance;
    uint64_t before = i->calling;
    jmp_buf *outer = i->out;
    jmp_buf here;
    volatile int ended = 0;

    i->out = &here;
    i->calling = os_get_thread_id();
    i->refused = 0;
    if (setjmp(here)) {
        ended = 1;
        snprintf(err->text, sizeof(err->text), "trapped: %s", i->why);
    } else
        *status = run(f);
    i->timeout = 0;
    i->calling = before;
    i->out = outer;
    return (ended);
}
