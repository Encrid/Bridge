/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge.
**
** All rights reserved.
**
** tests/modules/native/nsparse/nsparse_heard.c -- what the observers of
** nsparse hear, said in the log. A build for a measurement has none.
*/

#include "nsparse.h"

#if !defined(BIG) && !defined(FAT)
uint32_t nsparse_on_s;
uint32_t nsparse_on_both;

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
** heard_s
**
** Says what the observer of S heard.
*/
BRIDGE_NATIVE(heard_s)
{
    say_heard(api, nsparse_on_s, "heard of S: entity, events");
    return (0);
}

/*
** heard_both
**
** Says what the observer of A and S heard.
*/
BRIDGE_NATIVE(heard_both)
{
    say_heard(api, nsparse_on_both, "heard of A and S: entity, events");
    return (0);
}

#endif
