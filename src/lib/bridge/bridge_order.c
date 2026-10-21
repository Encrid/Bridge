/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "bridge_inside.h"
#include "say.h"
#include "schedule.h"

static int roll_back(bridge_world_t *w, schedule_pairs_t *saved, char *why,
    size_t len)
{
    say_error(w, "[order] is unchanged: %s", why);
    schedule_free_pairs(&w->order);
    w->order = *saved;
    (void)schedule_build(w, why, len);
    return (1);
}

int bridge_set_order_now(bridge_world_t *w, const bridge_order_t *pairs,
    uint32_t count)
{
    schedule_pairs_t saved;
    char why[256] = "";

    if (bridge_check_call(w, nullptr))
        return (1);
    saved = w->order;
    w->order = (schedule_pairs_t){};
    if (schedule_order(w, pairs, count)) {
        schedule_free_pairs(&w->order);
        w->order = saved;
        return (1);
    }
    if (schedule_build(w, why, sizeof(why)))
        return (roll_back(w, &saved, why, sizeof(why)));
    schedule_free_pairs(&saved);
    return (0);
}

int bridge_set_order(bridge_world_t *w, const bridge_order_t *pairs,
    uint32_t count)
{
    bool owner;
    int result;

    if (!bridge_enter(w, &owner) || !owner)
        return (1);
    result = bridge_set_order_now(w, pairs, count);
    bridge_leave(w, owner);
    return (result);
}
