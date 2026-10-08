/*
** Copyright (C) 2026 Bridge Owner
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/modules/wasm/events.c -- one observer of all three events, and a
** system that causes each in turn: an attach to an entity without the
** component, an attach to one with it, and a detach.
*/

#include "bridge_wasm.h"
#include "game.h"
#include "wasm_idle.h"

static uint32_t marked;
static uint32_t watching;
static uint32_t runs;
static uint64_t entity;

/*
** poke
**
** Attaches the component on its first two passes, and detaches it on the
** third.
*/
BRIDGE_EXPORT(poke) int poke(void)
{
    marked_t m = { runs + 1 };

    if (runs < 2)
        BRIDGE_ATTACH(entity, marked, m);
    else if (runs == 2)
        bridge_detach(entity, marked);
    runs++;
    return (0);
}

/*
** any
**
** Says, for each trigger, which events it is of and the value its id leads
** to. An id that leads nowhere was removed, and 0xdead is said for it.
*/
BRIDGE_EXPORT(any) int any(void)
{
    uint32_t count = bridge_count_triggers(watching);
    uint32_t trigger;
    uint32_t at;
    uint32_t i = 0;

    while (i < count) {
        trigger = bridge_get_trigger(watching, i);
        at = bridge_get_ref(bridge_get_trigger_id(trigger), marked,
            sizeof(marked_t));
        BRIDGE_LOG(BRIDGE_LOG_INFO, "event",
            bridge_get_trigger_events(trigger),
            at ? BRIDGE_GET(marked_t, by, at) : 0xdead);
        i++;
    }
    return (0);
}

/*
** init
**
** Declares the component, the system that changes it and the observer of
** its three events, and spawns the entity.
*/
BRIDGE_EXPORT(init) int init(void)
{
    bridge_term_t terms[1] = {{ 0, BRIDGE_READ, BRIDGE_AND, 0 }};
    int none = 0;

    marked = BRIDGE_COMPONENT("game:Marked", marked_t, BRIDGE_STRIP,
        BRIDGE_TABLE);
    terms[0].slot = marked;
    BRIDGE_ADD_SYSTEM("poke", "update", &none, 0, &marked, 1);
    watching = BRIDGE_OBSERVER("any",
        BRIDGE_ON_ENTER | BRIDGE_ON_ATTACH | BRIDGE_ON_LEAVE, terms, 1,
        nullptr, 0);
    entity = bridge_spawn();
    return (0);
}
