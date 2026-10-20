/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "bridge_wasm.h"
#include "game.h"
#include "wasm_idle.h"

static uint32_t marked;
static uint32_t fixing;

BRIDGE_EXPORT(fix) int fix(void)
{
    uint32_t count = bridge_count_triggers(fixing);
    uint64_t id;
    uint64_t e;
    uint32_t i = 0;

    while (i < count) {
        id = bridge_get_trigger_id(bridge_get_trigger(fixing, i));
        e = bridge_find(id);
        if (bridge_is_alive(id))
            BRIDGE_SET(marked_t, by, marked, bridge_entity_table(e),
                bridge_entity_row(e), 5u);
        i++;
    }
    return (0);
}

BRIDGE_EXPORT(init) int init(void)
{
    bridge_term_t terms[1] = {{ 0, BRIDGE_WRITE, BRIDGE_AND, 0 }};

    marked = BRIDGE_COMPONENT("game:Marked", marked_t, BRIDGE_STRIP,
        BRIDGE_TABLE);
    terms[0].slot = marked;
    fixing = BRIDGE_OBSERVER("fix", BRIDGE_ON_ENTER, terms, 1, nullptr, 0, nullptr, 0);
    return (0);
}
