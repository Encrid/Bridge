/*
** Copyright (C) 2026 Bridge Owner
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** src/module_discard.c -- a module's record freed, with what it held.
*/

#include <stdlib.h>

#include "module.h"

static void free_system(bridge_t *b, bridge_system_t *s)
{
    if (s->matched)
        storage_forget(b->storage, s->matched);
    if (s->parked)
        b->parked--;
    free(s->asked);
    free(s->pending);
    free(s->name);
    free(s->full);
    free(s);
}

/*
** module_drop_systems
**
** Takes a module's systems and observers out of storage and frees them.
*/
void module_drop_systems(bridge_t *b, module_t *m)
{
    bridge_system_t *s;
    uint32_t kept = 0;
    uint32_t i = 0;

    while (i < b->system_count) {
        s = b->systems[i];
        if (s->m == m)
            free_system(b, s);
        else
            b->systems[kept++] = s;
        i++;
    }
    b->system_count = kept;
    m->system_count = 0;
    m->observer_count = 0;
}

static void free_list(char **list, uint32_t count)
{
    uint32_t i = 0;

    while (list && i < count) {
        free(list[i]);
        i++;
    }
    free(list);
}

static void free_record(module_t *m)
{
    free_list(m->read, m->read_count);
    free_list(m->phase_from, m->phase_names);
    free_list(m->write, m->write_count);
    free(m->phase_to);
    free(m->ids);
    free(m->owned);
    free(m->queue);
    free(m->marks);
    free(m->writable);
    free(m->loss_rows);
    free(m->systems);
    free(m->observers);
    free(m->declared);
    free(m->policy);
    free(m->may_write);
    free(m->identity);
    free(m->stem);
    free(m->ns);
    free(m);
}

/*
** module_discard
**
** Frees a module and what it held: its systems, its instance, its image,
** its private memory, and the record. The order is the backend seam's: a
** memory is released after the last instance it was handed to.
*/
void module_discard(bridge_t *b, module_t *m)
{
    module_drop_systems(b, m);
    if (m->instance)
        m->on->api.instance_destroy(m->instance);
    if (m->image)
        m->on->api.module_destroy(m->image);
    if (m->memory)
        m->on->api.memory_release(m->memory);
    if (m->priv)
        os_release(m->priv, m->priv_reserve);
    free_record(m);
}
