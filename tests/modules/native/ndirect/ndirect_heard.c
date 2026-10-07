/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/modules/native/ndirect/ndirect_heard.c -- what the observers of
** ndirect hear, said in the log. A build for a measurement has none.
*/

#include "ndirect.h"

#ifndef BIG
uint32_t ndirect_on_d;
uint32_t ndirect_on_both;

/*
** say_heard
**
** Says each thing an observer heard: the entity, and the events.
*/
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

/*
** heard_d
**
** Says what the observer of D heard.
*/
BRIDGE_NATIVE(heard_d)
{
    say_heard(api, ndirect_on_d, "heard of D: entity, events");
    return (0);
}

/*
** heard_both
**
** Says what the observer of A and D heard.
*/
BRIDGE_NATIVE(heard_both)
{
    say_heard(api, ndirect_on_both, "heard of A and D: entity, events");
    return (0);
}

#endif
