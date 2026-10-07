/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge.
**
** All rights reserved.
**
** tests/modules/wasm/fixer.c -- observes game:Marked being added and
** overwrites it, under a write grant. Beside the watcher, which reads it at
** the same barrier, that is a conflict only the deployment can order.
*/

#include "bridge_wasm.h"
#include "game.h"
#include "wasm_idle.h"

static uint32_t marked;
static uint32_t fixing;

/*
** fix
**
** Writes 5 over the component of every entity that triggered the observer
** and still has it.
*/
BRIDGE_EXPORT(fix) int fix(void)
{
    uint32_t count = bridge_count_triggers(fixing);
    uint32_t at;
    uint32_t i = 0;

    while (i < count) {
        at = bridge_get_ref(
            bridge_get_trigger_id(bridge_get_trigger(fixing, i)), marked,
            sizeof(marked_t));
        if (at)
            BRIDGE_SET(marked_t, by, at, 5u);
        i++;
    }
    return (0);
}

/*
** init
**
** Declares the component, and the observer that writes it.
*/
BRIDGE_EXPORT(init) int init(void)
{
    bridge_term_t terms[1] = {{ 0, BRIDGE_WRITE, BRIDGE_AND, 0 }};

    marked = BRIDGE_COMPONENT("game:Marked", marked_t, BRIDGE_STRIP,
        BRIDGE_TABLE);
    terms[0].slot = marked;
    fixing = BRIDGE_OBSERVER("fix", BRIDGE_ON_ENTER, terms, 1, nullptr, 0);
    return (0);
}
