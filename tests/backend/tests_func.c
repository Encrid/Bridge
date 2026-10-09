/*
** Copyright (C) 2026 Bridge Owner
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/backend/tests_func.c -- what answers an entry kept, a function of
** a module found, and a call made, which a move of the tests may end.
**
** signatures.md, *A call takes nothing and answers a status*
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tests_inside.h"

/*
** define_func
**
** Keeps what answers an entry of the ABI.
**
** signatures.md, *Imports are declared, exports are checked*
*/
BACKEND_EXPORT int define_func(backend_instance_t *i,
    const backend_entry_t *entry, backend_err_t *err)
{
    uint32_t k = 0;

    while (k < TESTS_ENTRIES && strcmp(entry->field, TESTS_ENTRY_NAMES[k]))
        k++;
    if (k == TESTS_ENTRIES || strcmp(entry->ns, BRIDGE_NAMESPACE)) {
        snprintf(err->text, sizeof(err->text), "the tests' backend has no "
            "entry '%.100s'", entry->field);
        return (-1);
    }
    i->fns[k] = entry->fn;
    i->uds[k] = entry->ud;
    return (0);
}

/*
** instance_get_func
**
** Finds a function of a module, once, among those the build compiled in
** under the module's name.
**
** signatures.md, *A call takes nothing and answers a status*
*/
BACKEND_EXPORT backend_func_t *instance_get_func(backend_instance_t *i,
    const char *name, backend_err_t *err)
{
    const tests_function_t *t = TESTS_FUNCTIONS;
    const tests_function_t *end = t + TESTS_FUNCTION_COUNT;
    backend_func_t *f;

    while (t < end && (strcmp(t->module, i->m->name) || strcmp(t->name, name)))
        t++;
    if (t == end) {
        snprintf(err->text, sizeof(err->text), "no function '%.100s' in the "
            "module '%.100s'", name, i->m->name);
        return (nullptr);
    }
    f = calloc(1, sizeof(*f));
    f->instance = i;
    f->fn = t->fn;
    f->next = i->funcs;
    i->funcs = f;
    return (f);
}

/*
** call
**
** Calls a function of a module, and answers the status it returned. A call
** made from inside a call returns to it. A move of the tests that ends the
** call comes back here, and the call answers that it did not return, and
** why. The timeout was for this call only.
*/
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
        *status = f->fn(&i->api);
    i->timeout = 0;
    i->calling = before;
    i->out = outer;
    return (ended);
}
