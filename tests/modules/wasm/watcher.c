/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge-ASM700.
**
** All rights reserved.
**
** tests/modules/wasm/watcher.c -- observes game:Marked being added, under
** a read grant, and says who it was added to.
*/

#include "bridge_wasm.h"
#include "game.h"
#include "wasm_idle.h"

static uint32_t marked;
static uint32_t seeing;

/*
** seen
**
** Says, for each trigger, which events it is of and the value the other
** module attached, followed from the id. 0xdead is said for an id that
** leads nowhere.
*/
BRIDGE_EXPORT(seen) int seen(void)
{
    uint32_t count = bridge_count_triggers(seeing);
    uint32_t trigger;
    uint32_t at;
    uint32_t i = 0;

    while (i < count) {
        trigger = bridge_get_trigger(seeing, i);
        at = bridge_get_ref(bridge_get_trigger_id(trigger), marked,
            sizeof(marked_t));
        BRIDGE_LOG(BRIDGE_LOG_INFO, "seen",
            bridge_get_trigger_events(trigger),
            at ? BRIDGE_GET(marked_t, by, at) : 0xdead);
        i++;
    }
    return (0);
}

/*
** init
**
** Declares the component, and the observer that reads it.
*/
BRIDGE_EXPORT(init) int init(void)
{
    bridge_term_t terms[1] = {{ 0, BRIDGE_READ, BRIDGE_AND, 0 }};

    marked = BRIDGE_COMPONENT("game:Marked", marked_t, BRIDGE_STRIP,
        BRIDGE_TABLE);
    terms[0].slot = marked;
    seeing = BRIDGE_OBSERVER("seen", BRIDGE_ON_ENTER, terms, 1, nullptr, 0);
    return (0);
}
