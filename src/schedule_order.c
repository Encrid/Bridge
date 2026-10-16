/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdlib.h>
#include <string.h>

#include "schedule.h"

static bool is_pair(const char *before, const char *after)
{
    if (!before || !after || !*before || !*after)
        return (false);
    return (strcmp(before, after) != 0);
}

static uint32_t name_system(schedule_pairs_t *pairs, const char *name)
{
    uint32_t at = 0;

    while (at < pairs->n && strcmp(pairs->names[at], name))
        at++;
    if (at == pairs->n) {
        pairs->names[at] = name;
        pairs->n++;
    }
    return (at);
}

static int add_pair(bridge_t *b, schedule_pairs_t *pairs,
    const bridge_order_t *pair, uint32_t i)
{
    uint32_t before;
    uint32_t after;

    if (!is_pair(pair->before, pair->after)) {
        say_error(b, "[order] pair %u does not name two systems", i);
        return (1);
    }
    b->before[i] = copy_text(pair->before, strlen(pair->before));
    b->after[i] = copy_text(pair->after, strlen(pair->after));
    b->order_count = i + 1;
    if (!b->before[i] || !b->after[i])
        return (bridge_lacks(b, nullptr, "the order a deployment states"));
    before = name_system(pairs, pair->before);
    after = name_system(pairs, pair->after);
    schedule_add_edge(&pairs->reach, before, after);
    return (0);
}

static int check_pairs(bridge_t *b, schedule_pairs_t *pairs)
{
    uint32_t i = 0;

    schedule_close_matrix(&pairs->reach);
    while (i < pairs->n && !schedule_reaches(&pairs->reach, i, i))
        i++;
    if (i == pairs->n)
        return (0);
    say_error(b, "[order] contradicts itself: '%s' would run before itself",
        pairs->names[i]);
    return (1);
}

int schedule_order(bridge_t *b, const bridge_order_t *order, uint32_t count)
{
    schedule_pairs_t pairs = {};
    int refused = !schedule_init_matrix(&pairs.reach, count * 2);
    uint32_t i = 0;

    pairs.names = calloc(count ? count * 2 : 1, sizeof(*pairs.names));
    b->before = calloc(count ? count : 1, sizeof(*b->before));
    b->after = calloc(count ? count : 1, sizeof(*b->after));
    if (refused || !pairs.names || !b->before || !b->after)
        refused = bridge_lacks(b, nullptr, "the order a deployment states");
    while (i < count && !refused) {
        refused = add_pair(b, &pairs, &order[i], i);
        i++;
    }
    if (!refused && count)
        refused = check_pairs(b, &pairs);
    free(pairs.names);
    free(pairs.reach.bits);
    return (refused);
}
