/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdlib.h>

#include "module.h"

static void free_system(bridge_world_t *w, bridge_system_t *s)
{
    if (s->matched)
        storage_forget(w->storage, s->matched);
    if (s->parked)
        w->parked--;
    free(s->asked);
    free(s->pending);
    free(s->name);
    free(s->full);
    free(s);
}

void module_drop_systems(bridge_world_t *w, module_t *m)
{
    bridge_system_t *s;
    uint32_t kept = 0;
    uint32_t i = 0;

    while (i < w->system_count) {
        s = w->systems[i];
        if (s->m == m)
            free_system(w, s);
        else
            w->systems[kept++] = s;
        i++;
    }
    w->system_count = kept;
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

void module_discard(bridge_world_t *w, module_t *m)
{
    module_drop_systems(w, m);
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
