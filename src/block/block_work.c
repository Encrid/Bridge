/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "block.h"

void block_note_work(bridge_world_t *w, module_t *m)
{
    if (m->working || !m->linked)
        return;
    m->working = true;
    m->next_work = w->work;
    w->work = m;
    w->work_mixed = true;
}

void block_keep_work(bridge_world_t *w)
{
    module_t **at = &w->work;
    module_t *m;

    while (*at) {
        m = *at;
        if (m->queue_len || m->had_triggers || m->ids_short) {
            at = &m->next_work;
            continue;
        }
        *at = m->next_work;
        m->working = false;
    }
}

void block_drop_work(bridge_world_t *w, const module_t *m)
{
    module_t **at = &w->work;

    if (!m->working)
        return;
    while (*at != m)
        at = &(*at)->next_work;
    *at = m->next_work;
}

void block_join_work(bridge_world_t *w, module_t *m)
{
    m->place = w->places++;
    m->linked = true;
    block_note_work(w, m);
}

void block_take_place(bridge_world_t *w, module_t *n, const module_t *old)
{
    module_t **link = &w->modules;

    while (*link != old)
        link = &(*link)->next;
    n->next = old->next;
    *link = n;
    n->place = old->place;
    n->linked = true;
    block_note_work(w, n);
}
