/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge-ASM700.
**
** All rights reserved.
**
** backends/wasm/wasm_engine.c -- the clock: one armed deadline at a time,
** and no tick period.
**
** wasmtime.md, *Deadlines*
*/

#include "wasm_inside.h"

static void read_armed(backend_t *b, wasm_armed_t *armed)
{
    os_mutex_lock(b->lock);
    armed->active = b->active;
    armed->until = b->until;
    armed->engine = b->ticking;
    armed->quit = b->quit;
    os_mutex_unlock(b->lock);
}

/*
** wait_until
**
** Waits until a time or until the backend's event is set, whichever comes
** first. Answers true, without waiting, if the time has come.
*/
static bool wait_until(backend_t *b, uint64_t until)
{
    uint64_t now = os_get_now();

    if (now >= until)
        return (true);
    os_event_wait(b->wake, until - now);
    return (false);
}

/*
** wasm_watch
**
** The watchdog's thread. What is running is read afresh every time round,
** under the lock, and the event only says "look again". So an event that
** was set twice before it was waited for once loses nothing: two lifecycle
** calls back to back, a reload and then a shutdown, are exactly that. A
** call that is out of time traps at its next check, and its end is what
** wakes this.
*/
void wasm_watch(void *arg)
{
    backend_t *b = arg;
    wasm_armed_t armed;
    uint64_t interrupted = 0;

    read_armed(b, &armed);
    while (!armed.quit) {
        if (!armed.active || armed.active == interrupted) {
            os_event_wait(b->wake, OS_FOREVER);
        } else if (wait_until(b, armed.until)) {
            wasmtime_engine_increment_epoch(armed.engine);
            interrupted = armed.active;
        }
        read_armed(b, &armed);
    }
}
