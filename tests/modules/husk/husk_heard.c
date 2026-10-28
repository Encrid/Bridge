/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "husk.h"

static void count_events(uint32_t observer, uint32_t event, uint32_t *n)
{
    uint32_t count = bridge_count_triggers(observer);
    uint32_t i = 0;

    while (i < count) {
        if (bridge_get_trigger_events(bridge_get_trigger(observer, i)) & event)
            (*n)++;
        i++;
    }
}

BRIDGE_EXPORT(heard_body) int heard_body(void)
{
    count_events(husk_on_body, BRIDGE_ON_LEAVE, &husk_heard[0]);
    return (0);
}

BRIDGE_EXPORT(heard_handle) int heard_handle(void)
{
    count_events(husk_on_handle, BRIDGE_ON_LEAVE, &husk_heard[1]);
    return (0);
}

BRIDGE_EXPORT(heard_lone) int heard_lone(void)
{
    count_events(husk_on_lone, BRIDGE_ON_ENTER, &husk_heard[2]);
    return (0);
}
