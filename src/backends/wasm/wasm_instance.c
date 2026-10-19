/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdlib.h>
#include <string.h>

#include "wasm_inside.h"

BACKEND_EXPORT void instance_destroy(backend_instance_t *i)
{
    backend_func_t *f = i->funcs;

    while (f) {
        i->funcs = f->next;
        free(f);
        f = i->funcs;
    }
    wasmtime_linker_delete(i->linker);
    wasmtime_store_delete(i->store);
    free(i);
}

static void open_store(backend_instance_t *i, backend_module_t *m)
{
    i->b = m->b;
    i->m = m;
    i->engine = wasm_get_engine(m->b, m->kind);
    i->store = wasmtime_store_new(i->engine, nullptr, nullptr);
    i->ctx = wasmtime_store_context(i->store);
    i->linker = wasmtime_linker_new(i->engine);
    if (m->kind & WASM_METERED)
        wasm_drop(wasmtime_context_set_fuel(i->ctx, UINT64_MAX));
    if (m->kind & WASM_INTERRUPTIBLE)
        wasmtime_context_set_epoch_deadline(i->ctx, WASM_FAR);
}

static wasmtime_error_t *give_memory(backend_instance_t *i,
    const backend_memory_import_t *mi, wasm_trap_t **trap)
{
    wasmtime_extern_t item;
    wasmtime_error_t *error = wasm_adopt_into(i, mi->memory, &item, trap);

    if (error || *trap)
        return (error);
    return (wasmtime_linker_define(i->linker, i->ctx, mi->ns, strlen(mi->ns),
        mi->field, strlen(mi->field), &item));
}

BACKEND_EXPORT backend_instance_t *instance_create(backend_module_t *m,
    const backend_memory_import_t *memories, uint32_t count,
    backend_err_t *err)
{
    backend_instance_t *i = calloc(1, sizeof(*i));
    wasmtime_error_t *error = nullptr;
    wasm_trap_t *trap = nullptr;
    uint32_t k = 0;

    open_store(i, m);
    while (k < count && !error && !trap) {
        error = give_memory(i, &memories[k], &trap);
        k++;
    }
    if (!error && !trap)
        return (i);
    wasm_fail(err, "cannot give a module the memory it imports", error, trap);
    instance_destroy(i);
    return (nullptr);
}

BACKEND_EXPORT void instance_set_memory_limit(backend_instance_t *i,
    uint64_t bytes)
{
    wasmtime_store_limiter(i->store, (int64_t)bytes, -1, -1, -1, -1);
}
