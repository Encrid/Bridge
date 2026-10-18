/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "nsparse.h"

#if !defined(BIG) && !defined(FAT)
uint32_t nsparse_on_s;
uint32_t nsparse_on_both;

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

BRIDGE_NATIVE(heard_s)
{
    say_heard(api, nsparse_on_s, "heard of S: entity, events");
    return (0);
}

BRIDGE_NATIVE(heard_both)
{
    say_heard(api, nsparse_on_both, "heard of A and S: entity, events");
    return (0);
}

#endif
