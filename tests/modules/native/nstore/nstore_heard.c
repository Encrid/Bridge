/*
** Copyright (C) 2026 Bridge Owner
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/modules/native/nstore/nstore_heard.c -- what an observer of the
** plain build of nstore hears: who, and how. These are the observers of one
** component.
*/

#include "nstore.h"

#if !defined(BIG) && !defined(TABLES)
uint32_t nstore_on_a;
uint32_t nstore_on_b;

/*
** get_place
**
** Answers where an id sits among the module's own, so that nothing said
** depends on how ids are numbered: the place of a current one, 100 and the
** place of one it destroyed, and 999 for one that is neither, which was
** destroyed longer ago or was never the module's.
*/
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

/*
** nstore_say_heard
**
** Says each thing `observer` heard, under the text `what`: who, and the
** events. Who is where the id sits among the module's own, so that nothing
** said depends on how ids are numbered. Answers 0.
*/
int nstore_say_heard(const bridge_api_t *api, const char *what,
    uint32_t observer)
{
    const bridge_list_t *list = bridge_native_get_list(api, observer);
    const bridge_trigger_t *t;
    uint32_t i = 0;

    t = (const bridge_trigger_t *)(api->private_base + list->at);
    while (i < list->count) {
        say(api, what, get_place(t[i].id), t[i].events);
        i++;
    }
    return (0);
}

/*
** heard_a
**
** Says what the observer of A heard.
*/
BRIDGE_NATIVE(heard_a)
{
    return (nstore_say_heard(api, "A: who, events", nstore_on_a));
}

/*
** heard_b
**
** Says what the observer of B heard.
*/
BRIDGE_NATIVE(heard_b)
{
    return (nstore_say_heard(api, "B: who, events", nstore_on_b));
}

#endif
