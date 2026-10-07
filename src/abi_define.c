/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge-ASM700.
**
** All rights reserved.
**
** src/abi_define.c -- the eight imports of the ABI, defined for a module's
** instance with the signature each is declared with.
**
** signatures.md, *Imports are declared, exports are checked*
*/

#include "abi.h"

constexpr uint32_t ABI_ENTRY_COUNT = 8;
constexpr enum backend_type I = BACKEND_I32;
constexpr enum backend_type L = BACKEND_I64;

static const abi_entry_t ABI_ENTRIES[ABI_ENTRY_COUNT] = {
    { "declare", abi_declare, { I }, 1, { I }, 1 },
    { "add_system", abi_add_system, { I }, 1, { I }, 1 },
    { "observe", abi_observe, { I }, 1, { I }, 1 },
    { "spawn", abi_spawn, {}, 0, { L }, 1 },
    { "destroy", abi_destroy, { L }, 1, {}, 0 },
    { "attach", abi_attach, { L, I, I }, 3, {}, 0 },
    { "detach", abi_detach, { L, I }, 2, {}, 0 },
    { "alloc", abi_alloc, { I }, 1, { I }, 1 }
};

/*
** abi_define
**
** Defines the eight imports of the ABI for a module's instance, each handed
** the module when it is called.
*/
int abi_define(module_t *m, backend_err_t *err)
{
    const abi_entry_t *e = ABI_ENTRIES;
    backend_sig_t sig = {};
    backend_entry_t entry = { .ns = BRIDGE_NAMESPACE, .ud = m, .sig = &sig };
    uint32_t i = 0;

    while (i < ABI_ENTRY_COUNT) {
        sig.params = e[i].params;
        sig.param_count = e[i].param_count;
        sig.results = e[i].results;
        sig.result_count = e[i].result_count;
        entry.field = e[i].name;
        entry.fn = e[i].fn;
        if (m->on->api.define_func(m->instance, &entry, err))
            return (1);
        i++;
    }
    return (0);
}
