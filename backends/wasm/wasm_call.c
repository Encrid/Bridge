/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge.
**
** All rights reserved.
**
** backends/wasm/wasm_call.c -- a call into a module, under its limits.
**
** wasmtime.md, *Deadlines*
*/

#include "wasm_inside.h"

/*
** instance_set_work_limit
**
** States how much work an instance's next calls may do, in units of fuel.
*/
BACKEND_EXPORT void instance_set_work_limit(backend_instance_t *i,
    uint64_t units)
{
    if (i->m->kind & WASM_METERED)
        wasm_drop(wasmtime_context_set_fuel(i->ctx, units));
}

/*
** instance_set_timeout
**
** States how long an instance's next call may take, in nanoseconds.
*/
BACKEND_EXPORT void instance_set_timeout(backend_instance_t *i, uint64_t nanos)
{
    i->deadline = nanos;
}

/*
** arm
**
** Gives the watchdog a call to time, of `nanos` nanoseconds, or states that
** the next call has no deadline it reaches.
*/
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

/*
** call
**
** Calls a function of a module, and answers the status it returned. A
** module compiled without the check cannot be interrupted, and was not
** asked to be.
**
** signatures.md, *A call takes nothing and answers a status*
*/
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
