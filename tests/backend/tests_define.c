/*
** Copyright (C) 2026 Bridge Owner
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/backend/tests_define.c -- what answers an entry kept, once what it
** is declared to take and to answer has been held against the header.
**
** signatures.md, *Imports are declared, exports are checked*
*/

#include <stdio.h>
#include <string.h>

#include "tests_inside.h"

/*
** What each entry takes and answers, as bridge_abi.h states it, in the
** order of their names.
*/
constexpr enum backend_type I = BACKEND_I32;
constexpr enum backend_type L = BACKEND_I64;

static const tests_shape_t TESTS_SHAPES[TESTS_ENTRIES] = {
    { { I }, 1, I },
    { { I }, 1, I },
    { { I }, 1, I },
    { {}, 0, L },
    { { L }, 1, 0 },
    { { L, I, I }, 3, 0 },
    { { L, I }, 2, 0 },
    { { I }, 1, I }
};

/*
** is_shaped
**
** Answers whether a signature is the one the header states for an entry.
*/
static bool is_shaped(const tests_shape_t *shape, const backend_sig_t *sig)
{
    uint32_t results = shape->result ? 1 : 0;
    uint32_t k = 0;

    if (sig->param_count != shape->count || sig->result_count != results)
        return (false);
    while (k < shape->count && sig->params[k] == shape->params[k])
        k++;
    if (k < shape->count)
        return (false);
    return (!results || sig->results[0] == shape->result);
}

/*
** define_func
**
** Keeps what answers an entry of the ABI, and refuses one that is declared
** with another signature than bridge_abi.h states: every load of a module
** of the tests then shows that Bridge declares each as stated. One this
** backend has no place for is kept nowhere: no module of the tests could
** call it.
**
** signatures.md, *Imports are declared, exports are checked*
*/
BACKEND_EXPORT int define_func(backend_instance_t *i,
    const backend_entry_t *entry, backend_err_t *err)
{
    uint32_t k = 0;

    while (k < TESTS_ENTRIES && strcmp(entry->field, TESTS_ENTRY_NAMES[k]))
        k++;
    if (k == TESTS_ENTRIES || strcmp(entry->ns, BRIDGE_NAMESPACE))
        return (0);
    if (!is_shaped(&TESTS_SHAPES[k], entry->sig)) {
        snprintf(err->text, sizeof(err->text), "the entry '%.100s' is "
            "declared with another signature than bridge_abi.h states",
            entry->field);
        return (-1);
    }
    i->fns[k] = entry->fn;
    i->uds[k] = entry->ud;
    return (0);
}
