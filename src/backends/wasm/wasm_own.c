/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdlib.h>

#include "wasm_inside.h"

static uint8_t *get_own(void *env, size_t *size, size_t *capacity)
{
    wasm_own_t *m = env;

    *size = m->size;
    *capacity = m->reserve;
    return (m->base);
}

static wasmtime_error_t *grow_own(void *env, size_t new_size)
{
    wasm_own_t *m = env;

    if (new_size > m->reserve || os_commit(m->base, new_size))
        return (wasmtime_error_new("out of address space for a module's own "
            "memory"));
    m->size = new_size;
    return (nullptr);
}

static void free_own(void *env)
{
    wasm_own_t *m = env;

    os_release(m->base, m->reserve + (size_t)WASM_GUARD);
    free(m);
}

wasmtime_error_t *wasm_make_own(size_t minimum, wasmtime_linear_memory_t *ret)
{
    wasm_own_t *m = malloc(sizeof(*m));

    m->reserve = (size_t)WASM_RESERVATION;
    m->size = minimum;
    m->base = os_reserve((size_t)(WASM_RESERVATION + WASM_GUARD), minimum);
    if (!m->base) {
        free(m);
        return (wasmtime_error_new("out of address space for a module's own "
            "memory"));
    }
    ret->env = m;
    ret->get_memory = get_own;
    ret->grow_memory = grow_own;
    ret->finalizer = free_own;
    return (nullptr);
}
