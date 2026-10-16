/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "schedule.h"

static bool changes_it(const bridge_system_t *s, const declared_type_t *type)
{
    uint32_t i = 0;

    while (i < s->change_count && s->changes[i] != type)
        i++;
    return (i < s->change_count);
}

static const char *find_changed(const bridge_system_t *a,
    const bridge_system_t *b)
{
    uint32_t i = 0;

    while (i < a->change_count && !changes_it(b, a->changes[i]))
        i++;
    if (i == a->change_count)
        return (nullptr);
    return (a->changes[i]->name);
}

static bool touches_it(const bridge_system_t *s, const bridge_system_t *other,
    uint32_t i)
{
    const declared_type_t *type = other->term[i].touches;
    bool writes = other->term[i].writes;
    uint32_t j = 0;

    if (!type)
        return (false);
    while (j < s->terms) {
        if (s->term[j].touches == type && (writes || s->term[j].writes))
            return (true);
        j++;
    }
    return (false);
}

static const char *find_touched(const bridge_system_t *a,
    const bridge_system_t *b)
{
    uint32_t i = 0;

    while (i < a->terms && !touches_it(b, a, i))
        i++;
    if (i == a->terms)
        return (nullptr);
    return (a->term[i].touches->name);
}

const char *schedule_find_conflict(const bridge_system_t *a,
    const bridge_system_t *b, bool *changing)
{
    const char *shared = find_changed(a, b);

    *changing = shared != nullptr;
    if (shared)
        return (shared);
    return (find_touched(a, b));
}
