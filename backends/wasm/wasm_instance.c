/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge-ASM700.
**
** All rights reserved.
**
** backends/wasm/wasm_instance.c -- an instance made and freed.
*/

#include <stdlib.h>
#include <string.h>

#include "wasm_inside.h"

/*
** instance_destroy
**
** Frees an instance, with the functions found in it.
*/
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

/*
** open_store
**
** Gives an instance its store and its linker. A metered store with no fuel
** traps at its first instruction, and an interruptible one with no deadline
** at the first increment: a holder runs nothing, but it is instantiated in
** this store.
*/
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

/*
** instance_create
**
** Makes an instance of a module, with the memories it imports.
*/
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

/*
** instance_set_memory_limit
**
** Limits how many bytes an instance's own memories may come to.
*/
BACKEND_EXPORT void instance_set_memory_limit(backend_instance_t *i,
    uint64_t bytes)
{
    wasmtime_store_limiter(i->store, (int64_t)bytes, -1, -1, -1, -1);
}
