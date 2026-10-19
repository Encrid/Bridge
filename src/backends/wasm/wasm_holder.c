/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdlib.h>
#include <string.h>

#include "wasm_inside.h"

static size_t write_leb(uint8_t *out, uint64_t v)
{
    size_t n = 0;

    while (v >= 0x80) {
        out[n] = (uint8_t)(v & 0x7f) | 0x80;
        v >>= 7;
        n++;
    }
    out[n] = (uint8_t)v;
    return (n + 1);
}

static wasmtime_error_t *build_holder(wasm_engine_t *engine, uint64_t pages,
    wasmtime_module_t **holder)
{
    uint8_t wasm[64] = "\0asm\1\0\0\0\5";
    uint8_t limits[24] = { 1, 1 };
    size_t l = 2;
    size_t n = 10;

    l += write_leb(limits + l, pages);
    l += write_leb(limits + l, pages);
    wasm[9] = (uint8_t)l;
    memcpy(wasm + n, limits, l);
    n += l;
    memcpy(wasm + n, "\7\5\1\1m\2\0", 7);
    n += 7;
    return (wasmtime_module_new(engine, wasm, n, holder));
}

wasmtime_error_t *wasm_adopt_into(backend_instance_t *i, backend_memory_t *mem,
    wasmtime_extern_t *item, wasm_trap_t **trap)
{
    wasmtime_module_t **holder = &mem->holder[i->m->kind];
    wasmtime_instance_t instance;
    wasmtime_error_t *error = nullptr;

    os_mutex_lock(i->b->lock);
    if (!*holder)
        error = build_holder(i->engine, mem->len / WASM_PAGE, holder);
    i->b->adopting = mem;
    if (!error)
        error = wasmtime_instance_new(i->ctx, *holder, nullptr, 0, &instance,
            trap);
    i->b->adopting = nullptr;
    os_mutex_unlock(i->b->lock);
    if (error || *trap)
        return (error);
    if (wasmtime_instance_export_get(i->ctx, &instance, "m", 1, item))
        return (nullptr);
    return (wasmtime_error_new("the holder module exports no memory"));
}

BACKEND_EXPORT backend_memory_t *memory_adopt([[maybe_unused]] backend_t *b,
    void *ptr, uint64_t len, backend_err_t *err)
{
    backend_memory_t *m;

    if (!ptr || !len || len % WASM_PAGE) {
        wasm_fail(err, "an adopted block must be a whole number of 64 "
            "KiB pages", nullptr, nullptr);
        return (nullptr);
    }
    m = calloc(1, sizeof(*m));
    m->ptr = ptr;
    m->len = len;
    return (m);
}

BACKEND_EXPORT void memory_release(backend_memory_t *m)
{
    int k = 0;

    while (k < WASM_ENGINES) {
        if (m->holder[k])
            wasmtime_module_delete(m->holder[k]);
        k++;
    }
    free(m);
}
