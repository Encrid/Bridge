/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "wasm_inside.h"

BACKEND_EXPORT void instance_set_work_limit(backend_instance_t *i,
    uint64_t units)
{
    if (i->m->kind & WASM_METERED)
        wasm_drop(wasmtime_context_set_fuel(i->ctx, units));
}

BACKEND_EXPORT void instance_set_timeout(backend_instance_t *i, uint64_t nanos)
{
    i->deadline = nanos;
}

static void arm(backend_instance_t *i, uint64_t nanos)
{
    backend_t *b = i->b;

    wasmtime_context_set_epoch_deadline(i->ctx, nanos ? 1 : WASM_FAR);
    if (!nanos)
        return;
    os_mutex_lock(b->lock);
    b->calls++;
    b->active = b->calls;
    b->until = os_get_now() + nanos;
    b->ticking = i->engine;
    os_mutex_unlock(b->lock);
    os_event_set(b->wake);
}

static void disarm(backend_t *b)
{
    os_mutex_lock(b->lock);
    b->active = 0;
    os_mutex_unlock(b->lock);
    os_event_set(b->wake);
}

BACKEND_EXPORT int call(backend_func_t *f, int32_t *status, backend_err_t *err)
{
    backend_instance_t *i = f->instance;
    bool timed = (i->m->kind & WASM_INTERRUPTIBLE) != 0;
    uint64_t nanos = timed ? i->deadline : 0;
    wasmtime_val_t result;
    wasm_trap_t *trap = nullptr;
    wasmtime_error_t *error;

    i->deadline = 0;
    if (timed)
        arm(i, nanos);
    error = wasmtime_func_call(i->ctx, &f->func, nullptr, 0, &result, 1, &trap);
    if (nanos)
        disarm(i->b);
    if (error || trap)
        return (wasm_fail(err, "trapped", error, trap));
    *status = result.of.i32;
    return (0);
}
