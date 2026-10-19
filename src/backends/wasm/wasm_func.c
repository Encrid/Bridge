/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "wasm_inside.h"

BACKEND_EXPORT int define_func(backend_instance_t *i,
    const backend_entry_t *entry, backend_err_t *err)
{
    const backend_sig_t *sig = entry->sig;
    wasm_valtype_vec_t params;
    wasm_valtype_vec_t results;
    wasm_functype_t *ty;
    wasmtime_error_t *error = nullptr;

    if (i->instantiated || sig->param_count > 16 || sig->result_count > 4)
        return (wasm_fail(err, "define_func: after the module's first "
            "function, or too wide", nullptr, nullptr));
    wasm_make_types(&params, sig->params, sig->param_count);
    wasm_make_types(&results, sig->results, sig->result_count);
    ty = wasm_functype_new(&params, &results);
    error = wasmtime_linker_define_func_unchecked(i->linker, entry->ns,
        strlen(entry->ns), entry->field, strlen(entry->field), ty,
        wasm_call_offered, wasm_make_offered(entry), free);
    wasm_functype_delete(ty);
    if (error)
        return (wasm_fail(err, "cannot define a function", error, nullptr));
    return (0);
}

static int instantiate(backend_instance_t *i, backend_err_t *err)
{
    wasm_trap_t *trap = nullptr;
    wasmtime_error_t *error;

    if (i->instantiated)
        return (0);
    error = wasmtime_linker_instantiate(i->linker, i->ctx, i->m->module,
        &i->instance, &trap);
    if (error || trap)
        return (wasm_fail(err, "cannot instantiate", error, trap));
    i->instantiated = 1;
    return (0);
}

static bool is_shaped(backend_instance_t *i, const wasmtime_func_t *func)
{
    wasm_functype_t *ty = wasmtime_func_type(i->ctx, func);
    const wasm_valtype_vec_t *results = wasm_functype_results(ty);
    bool shaped = !wasm_functype_params(ty)->size && results->size == 1
        && wasm_valtype_kind(results->data[0]) == WASM_I32;

    wasm_functype_delete(ty);
    return (shaped);
}

static int find_export(backend_instance_t *i, const char *name,
    wasmtime_extern_t *item, backend_err_t *err)
{
    constexpr size_t room = sizeof(err->text);
    bool found = wasmtime_instance_export_get(i->ctx, &i->instance, name,
        strlen(name), item) && item->kind == WASMTIME_EXTERN_FUNC;

    if (found && is_shaped(i, &item->of.func))
        return (0);
    if (!err)
        return (1);
    if (found)
        snprintf(err->text, room, "its export '%.100s' is not () -> i32", name);
    else
        snprintf(err->text, room, "no exported function '%s'", name);
    return (1);
}

BACKEND_EXPORT backend_func_t *instance_get_func(backend_instance_t *i,
    const char *name, backend_err_t *err)
{
    wasmtime_extern_t item;
    backend_func_t *f;

    if (instantiate(i, err) || find_export(i, name, &item, err))
        return (nullptr);
    f = calloc(1, sizeof(*f));
    f->instance = i;
    f->func = item.of.func;
    f->next = i->funcs;
    i->funcs = f;
    return (f);
}
