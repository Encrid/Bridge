/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "ndirect.h"

#ifndef BIG
uint32_t ndirect_on_d;
uint32_t ndirect_on_both;

static void say_heard(const bridge_api_t *api, uint32_t observer,
    const char *text)
{
    const bridge_list_t *list = bridge_native_get_list(api, observer);
    const bridge_trigger_t *trigger;
    uint32_t i = 0;

    trigger = (const bridge_trigger_t *)(api->private_base + list->at);
    while (i < list->count) {
        say(api, text, (uint32_t)trigger[i].id, trigger[i].events);
        i++;
    }
}

BRIDGE_NATIVE(heard_d)
{
    say_heard(api, ndirect_on_d, "heard of D: entity, events");
    return (0);
}

BRIDGE_NATIVE(heard_both)
{
    say_heard(api, ndirect_on_both, "heard of A and D: entity, events");
    return (0);
}

#endif
