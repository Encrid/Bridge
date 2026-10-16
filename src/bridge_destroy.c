/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdlib.h>

#include "bridge_inside.h"
#include "say.h"

static void end_modules(bridge_t *b)
{
    module_t *m = b->modules;

    while (m) {
        module_shut_down(b, m);
        m = m->next;
    }
    while (b->modules) {
        m = b->modules;
        b->modules = m->next;
        module_discard(b, m);
    }
}

static void free_schedule(bridge_t *b)
{
    free(b->systems);
    free(b->run_jobs);
    free(b->run_handed);
    free(b->run_lists);
    free(b->run_callers);
    free(b->build_in);
    free(b->build_bits);
}

static void free_names(bridge_t *b)
{
    uint32_t i = 0;

    while (i < b->order_count) {
        free(b->before[i]);
        free(b->after[i]);
        i++;
    }
    free(b->before);
    free(b->after);
    i = 0;
    while (i < b->phase_count) {
        free(b->phases[i]);
        i++;
    }
    free(b->phases);
}

static void close_backends(bridge_t *b)
{
    open_backend_t *o;
    uint32_t i = 0;

    while (i < b->backend_count) {
        o = &b->backends[i];
        if (o->be)
            o->api.backend_destroy(o->be);
        if (o->lib && (o->capabilities & BACKEND_UNLOAD))
            os_lib_close(o->lib);
        free(o->name);
        i++;
    }
    free(b->backends);
}

int bridge_destroy(bridge_t *b)
{
    if (!b)
        return (0);
    if (bridge_check_entry(b))
        return (1);
    end_modules(b);
    free_schedule(b);
    block_destroy(b);
    pool_destroy(b->pool);
    free_names(b);
    close_backends(b);
    free(b);
    return (0);
}
