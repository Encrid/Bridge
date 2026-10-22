/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "store.h"

#if !defined(BIG) && !defined(TABLES)
uint32_t nstore_on_a;
uint32_t nstore_on_b;

static uint32_t get_place(uint64_t id)
{
    uint32_t i = 0;

    while (i < COUNT) {
        if (nstore_ids[i] == id)
            return (i);
        i++;
    }
    i = 0;
    while (i < COUNT) {
        if (nstore_old[i] == id)
            return (100 + i);
        i++;
    }
    return (999);
}

int nstore_say_heard(const test_api_t *api, const char *what,
    uint32_t observer)
{
    const bridge_list_t *list = test_get_list(api, observer);
    const bridge_trigger_t *t;
    uint32_t i = 0;

    t = (const bridge_trigger_t *)(api->private_base + list->at);
    while (i < list->count) {
        say(api, what, get_place(t[i].id), t[i].events);
        i++;
    }
    return (0);
}

TEST(heard_a)
{
    return (nstore_say_heard(api, "A: who, events", nstore_on_a));
}

TEST(heard_b)
{
    return (nstore_say_heard(api, "B: who, events", nstore_on_b));
}

#endif
