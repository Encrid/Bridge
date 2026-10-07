/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** backends/wasm/wasm_adopted.c -- a block Bridge owns, as a memory
** of wasmtime's, and the engines, which are made with what answers wasmtime
** for a memory.
**
** wasmtime.md, *Memory*
** wasmtime.md, *One engine per demand*
*/

#include "wasm_inside.h"

static uint8_t *get_adopted(void *env, size_t *size, size_t *capacity)
{
    backend_memory_t *m = env;

    *size = (size_t)m->len;
    *capacity = (size_t)m->len;
    return (m->ptr);
}

static wasmtime_error_t *grow_adopted(void *env, size_t new_size)
{
    const backend_memory_t *m = env;

    if (new_size == m->len)
        return (nullptr);
    return (wasmtime_error_new("an adopted memory does not grow"));
}

/*
** make_memory
**
** What wasmtime calls for every linear memory an instance defines: the
** adopted block its backend states, or a memory of the module's own. `env`
** is the backend. An adopted block has no finalizer: it is Bridge's
** to free. The signature is wasmtime's.
*/
static wasmtime_error_t *make_memory(void *env,
    [[maybe_unused]] const wasm_memorytype_t *ty, size_t minimum,
    [[maybe_unused]] size_t maximum, size_t reserved, size_t guard,
    wasmtime_linear_memory_t *ret)
{
    backend_memory_t *adopting = ((backend_t *)env)->adopting;

    if (reserved > WASM_RESERVATION || guard > WASM_GUARD)
        return (wasmtime_error_new("wasmtime asked for more address space "
            "than memory_get_shape states"));
    if (!adopting)
        return (wasm_make_own(minimum, ret));
    if (minimum != adopting->len)
        return (wasmtime_error_new("an adopted block is not the size its "
            "import declares"));
    ret->env = adopting;
    ret->get_memory = get_adopted;
    ret->grow_memory = grow_adopted;
    ret->finalizer = nullptr;
    return (nullptr);
}

/*
** make_engine
**
** Makes the engine of a kind. wasmtime initialises a memory from a
** copy-on-write image where the system has one, and asserts that such a
** memory is one it mapped itself. Every memory here comes from the creator,
** which is handed the backend, so there is no memory an image could be laid
** on.
*/
static wasm_engine_t *make_engine(backend_t *b, enum wasm_engine kind)
{
    wasm_config_t *config = wasm_config_new();
    wasmtime_memory_creator_t creator = { b, make_memory, nullptr };
    bool metered = (kind & WASM_METERED) != 0;
    bool interruptible = (kind & WASM_INTERRUPTIBLE) != 0;

    wasmtime_config_wasm_multi_memory_set(config, true);
    wasmtime_config_consume_fuel_set(config, metered);
    wasmtime_config_epoch_interruption_set(config, interruptible);
    wasmtime_config_memory_reservation_set(config, WASM_RESERVATION);
    wasmtime_config_memory_guard_size_set(config, WASM_GUARD);
    wasmtime_config_memory_may_move_set(config, false);
    wasmtime_config_memory_init_cow_set(config, false);
    wasmtime_config_host_memory_creator_set(config, &creator);
    return (wasm_engine_new_with_config(config));
}

/*
** wasm_get_engine
**
** Answers the engine of a kind, made at its first use.
*/
wasm_engine_t *wasm_get_engine(backend_t *b, enum wasm_engine kind)
{
    if (!b->engines[kind])
        b->engines[kind] = make_engine(b, kind);
    return (b->engines[kind]);
}
