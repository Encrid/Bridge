/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "module.h"

static void call_shutdown(bridge_world_t *w, module_t *m)
{
    module_call_t call = { m->shutdown, "shutdown", m->shutdown_timeout };
    say_t about = { m, nullptr, BRIDGE_ERR };
    backend_err_t err = {};

    if (module_call(m, &call, &err))
        say_about(w, &about, "module '%s': %s; its teardown continues",
            m->identity, err.text);
}

void module_shut_down(bridge_world_t *w, module_t *m)
{
    if (m->instance)
        call_shutdown(w, m);
    block_hand_out(w);
    storage_apply(w->storage);
    block_drain(w, m);
}

void module_shut_down_new(bridge_world_t *w, module_t *m)
{
    if (!m->shutdown)
        return;
    call_shutdown(w, m);
    block_drain(w, m);
}
