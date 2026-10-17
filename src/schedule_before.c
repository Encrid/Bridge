/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "schedule.h"

void schedule_join_before(bridge_system_t *s, const bridge_system_t *t)
{
    schedule_join_bits(s->stated_before, t->stated_before,
        schedule_get_words(t->slot));
    schedule_put_bit(s->stated_before, t->slot);
}

bool schedule_is_stated_before(const bridge_system_t *s,
    const bridge_system_t *t)
{
    return (s->slot < t->slot && schedule_has_bit(t->stated_before, s->slot));
}
