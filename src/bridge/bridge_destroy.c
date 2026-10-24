/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdlib.h>

#include "bridge_inside.h"
#include "say.h"
#include "schedule.h"

static void end_modules(bridge_world_t *w)
{
    module_t *m = w->modules;

    while (m) {
        module_shut_down(w, m);
        m = m->next;
    }
    while (w->modules) {
        m = w->modules;
        w->modules = m->next;
        module_discard(w, m);
    }
}

static void free_schedule(bridge_world_t *w)
{
    free(w->systems);
    free(w->run_jobs);
    free(w->run_handed);
    free(w->run_lists);
    free(w->run_callers);
    free(w->build_in);
    free(w->build_bits);
}

static void free_names(bridge_world_t *w)
{
    schedule_free_pairs(&w->order);
    for (uint32_t i = 0; i < w->phase_count; i++)
        free(w->phases[i]);
    free(w->phases);
}

static void close_backends(bridge_world_t *w)
{
    open_backend_t *o;
    uint32_t i = 0;

    while (i < w->backend_count) {
        o = &w->backends[i];
        if (o->be)
            o->api.backend_destroy(o->be);
        if (o->lib && (o->capabilities & BACKEND_UNLOAD))
            os_lib_close(o->lib);
        free(o->name);
        i++;
    }
    free(w->backends);
}

int bridge_destroy(bridge_world_t *w)
{
    bool owner;

    if (!w)
        return (0);
    if (!bridge_enter(w, &owner) || !owner)
        return (1);
    if (bridge_check_entry(w)) {
        bridge_leave(w, owner);
        return (1);
    }
    end_modules(w);
    free_schedule(w);
    block_destroy(w);
    pool_destroy(w->pool);
    free_names(w);
    close_backends(w);
    free(w);
    return (0);
}
