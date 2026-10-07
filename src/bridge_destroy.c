/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge-ASM700.
**
** All rights reserved.
**
** src/bridge_destroy.c -- a bridge unmade: its modules shut down, and
** everything else given back.
*/

#include <stdlib.h>

#include "bridge_inside.h"
#include "say.h"

/*
** end_modules
**
** Runs each module's shutdown, in the order they loaded, and then discards
** each: nothing here depends on anything, so no other order would mean more.
*/
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

/*
** free_names
**
** Frees the names the deployment gave: the orders of its systems, and its
** phases.
*/
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

/*
** close_backends
**
** Destroys each backend, and closes the library of one that says its library
** can go. One that brought a runtime in is left in the process, where the
** next bridge finds it again. A library that was opened and never became a
** backend said nothing, and is left too.
**
** capabilities.md, *Capabilities, so the swap is honest*
*/
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

/*
** bridge_destroy
**
** Every loaded module's shutdown, then everything else. 0 if the bridge is
** gone. A pinned module's shutdown is its code, and runs on its thread or
** not at all: it is refused before anything is touched, so the bridge is
** whole for the call that can.
**
** create.md, *Destroy*
*/
int bridge_destroy(bridge_t *b)
{
    const module_t *pinned = b ? bridge_find_pinned(b) : nullptr;

    if (!b)
        return (0);
    if (pinned) {
        say_error(b, "bridge_destroy is refused: module '%s' is pinned to "
            "the thread that loaded it, and this call comes from another",
            pinned->identity);
        return (1);
    }
    end_modules(b);
    free_schedule(b);
    block_destroy(b);
    pool_destroy(b->pool);
    free_names(b);
    close_backends(b);
    free(b);
    return (0);
}
