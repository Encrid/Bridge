/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "schedule.h"

static void link_use(bridge_system_t *s, declared_type_t *type)
{
    system_use_t *u = &s->use[s->use_count];

    u->s = s;
    u->type = type;
    u->next = type->uses;
    type->uses = u;
    s->use_count++;
}

void schedule_index_system(bridge_system_t *s)
{
    for (uint32_t i = 0; i < s->terms; i++)
        if (s->term[i].touches)
            link_use(s, s->term[i].touches);
    for (uint32_t i = 0; i < s->change_count; i++)
        link_use(s, s->changes[i]);
}

void schedule_unindex_system(bridge_system_t *s)
{
    system_use_t **at;

    for (uint32_t i = 0; i < s->use_count; i++) {
        at = &s->use[i].type->uses;
        while (*at != &s->use[i])
            at = &(*at)->next;
        *at = s->use[i].next;
    }
    s->use_count = 0;
}
