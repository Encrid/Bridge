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
    uint32_t colons = 0;
    size_t run = 0;

    if (!name || strchr(name, '@'))
        return (false);
    for (const char *c = name; *c; c++) {
        if (*c != ':') {
            run++;
            continue;
        }
        if (!run)
            return (false);
        colons++;
        run = 0;
    }
    return (colons == 2 && run && run <= NAME_MOST);
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

static int add_pair(bridge_world_t *w, const bridge_order_t *pair, uint32_t i)
{
    schedule_pairs_t *order = &w->order;
    uint32_t before;
    uint32_t after = 0;

    if (!is_full_name(pair->before) || !is_full_name(pair->after)
        || !strcmp(pair->before, pair->after)) {
        say_error(w, "[order] pair %u does not name two systems", i);
        return (1);
    }
    before = name_system(order, pair->before);
    if (before < order->n)
        after = name_system(order, pair->after);
    if (before == order->n || after == order->n)
        return (bridge_lacks(w, nullptr, "the order a deployment states"));
    order->pairs[2 * order->pair_count] = before;
    order->pairs[2 * order->pair_count + 1] = after;
    order->pair_count++;
    schedule_add_edge(&order->reach, before, after);
    return (0);
}

static int check_pairs(bridge_world_t *w, schedule_pairs_t *pairs)
{
    uint32_t i;

    schedule_close_matrix(&pairs->reach);
    i = schedule_get_cycle(&pairs->reach);
    if (i == pairs->reach.n)
        return (0);
    say_error(w, "[order] contradicts itself: '%s' would run before itself",
        pairs->names[i]);
    return (1);
}

int schedule_order(bridge_world_t *w, const bridge_order_t *order,
    uint32_t count)
{
    schedule_pairs_t *pairs = &w->order;
    int refused;
    uint32_t i = 0;

    if ((count && !order) || count > UINT32_MAX / 2u) {
        say_error(w, "[order] has %u pairs Bridge cannot read", count);
        return (1);
    }
    refused = !schedule_init_matrix(&pairs->reach, count * 2);
    pairs->names = calloc(count ? count * 2 : 1, sizeof(*pairs->names));
    pairs->pairs = calloc(count ? count * 2 : 1, sizeof(*pairs->pairs));
    if (refused || !pairs->names || !pairs->pairs)
        refused = bridge_lacks(w, nullptr, "the order a deployment states");
    while (i < count && !refused) {
        refused = add_pair(w, &order[i], i);
        i++;
    }
    if (!refused && count)
        refused = check_pairs(w, pairs);
    return (refused);
}
