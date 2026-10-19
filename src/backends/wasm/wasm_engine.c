/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
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

static bool wait_until(backend_t *b, uint64_t until)
{
    uint64_t now = os_get_now();

    if (now >= until)
        return (true);
    os_event_wait(b->wake, until - now);
    return (false);
}

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
