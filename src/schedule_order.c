/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdlib.h>
#include <string.h>

#include "schedule.h"

static bool is_full_name(const char *name)
{
    const char *colon = name ? strrchr(name, ':') : nullptr;
    size_t tail;

    if (!colon || strchr(name, '@'))
        return (false);
    tail = strlen(colon + 1);
    return (tail && tail <= NAME_MOST);
}

static uint32_t name_system(schedule_pairs_t *order, const char *name)
{
    uint32_t at = schedule_find_name(order, name);

    if (at < order->n)
        return (at);
    order->names[at] = copy_text(name, strlen(name));
    if (order->names[at])
        order->n++;
    return (at);
}

static int add_pair(bridge_t *b, const bridge_order_t *pair, uint32_t i)
{
    schedule_pairs_t *order = &b->order;
    uint32_t before;
    uint32_t after = 0;

    if (!is_full_name(pair->before) || !is_full_name(pair->after)
        || !strcmp(pair->before, pair->after)) {
        say_error(b, "[order] pair %u does not name two systems", i);
        return (1);
    }
    before = name_system(order, pair->before);
    if (before < order->n)
        after = name_system(order, pair->after);
    if (before == order->n || after == order->n)
        return (bridge_lacks(b, nullptr, "the order a deployment states"));
    schedule_add_edge(&order->reach, before, after);
    return (0);
}

static int check_pairs(bridge_t *b, schedule_pairs_t *pairs)
{
    uint32_t i;

    schedule_close_matrix(&pairs->reach);
    i = schedule_get_cycle(&pairs->reach);
    if (i == pairs->reach.n)
        return (0);
    say_error(b, "[order] contradicts itself: '%s' would run before itself",
        pairs->names[i]);
    return (1);
}

int schedule_order(bridge_t *b, const bridge_order_t *order, uint32_t count)
{
    schedule_pairs_t *pairs = &b->order;
    int refused = !schedule_init_matrix(&pairs->reach, count * 2);
    uint32_t i = 0;

    pairs->names = calloc(count ? count * 2 : 1, sizeof(*pairs->names));
    if (refused || !pairs->names)
        refused = bridge_lacks(b, nullptr, "the order a deployment states");
    while (i < count && !refused) {
        refused = add_pair(b, &order[i], i);
        i++;
    }
    if (!refused && count)
        refused = check_pairs(b, pairs);
    return (refused);
}
