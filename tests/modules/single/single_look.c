/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "single.h"
#include "measured.h"

volatile uint32_t nsingle_which;

TEST(look)
{
    const g_t *value = test_get_single(api, nsingle_g);

    if (value)
        say(api, "it has a value: gravity, wind", value->gravity,
            value->wind);
    else
        say(api, "it has no value", 0, 0);
    return (0);
}

TEST_MEASURED TEST(one)
{
    const g_t *value;
    uint32_t total = 0;
    uint32_t i = 0;

    while (i < ASKED) {
        value = test_get_single(api, nsingle_which);
        if (value)
            total += value->gravity;
        i++;
    }
    say(api, "read a million times: the sum", total, 0);
    return (0);
}

TEST(heard_g)
{
    const bridge_list_t *list = test_get_list(api, nsingle_on_g);
    const bridge_trigger_t *trigger;
    uint32_t i = 0;

    trigger = (const bridge_trigger_t *)(api->private_base + list->at);
    while (i < list->count) {
        say(api, "heard of it: the id, events", (uint32_t)trigger[i].id,
            trigger[i].events);
        i++;
    }
    return (0);
}
