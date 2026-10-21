/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "bits.h"
#include "schedule.h"

void schedule_join_before(bridge_system_t *s, const bridge_system_t *t)
{
    bits_or(s->stated_before, t->stated_before, bits_words(t->slot));
    bits_set(s->stated_before, t->slot);
}

bool schedule_is_stated_before(const bridge_system_t *s,
    const bridge_system_t *t)
{
    return (s->slot < t->slot && bits_has(t->stated_before, s->slot));
}
