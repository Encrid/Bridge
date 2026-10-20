/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "bridge_wasm.h"
#include "game.h"
#include "wasm_idle.h"

static uint32_t marked;
static uint32_t seeing;

BRIDGE_EXPORT(seen) int seen(void)
{
    uint32_t count = bridge_count_triggers(seeing);
    uint32_t trigger;
    uint64_t id;
    uint64_t e;
    uint32_t i = 0;

    while (i < count) {
        trigger = bridge_get_trigger(seeing, i);
        id = bridge_get_trigger_id(trigger);
        e = bridge_find(id);
        BRIDGE_LOG(BRIDGE_LOG_INFO, "seen",
            bridge_get_trigger_events(trigger),
            (bridge_is_alive(id)
                    && bridge_base(marked, bridge_entity_table(e)))
                ? BRIDGE_GET(marked_t, by, marked, bridge_entity_table(e),
                    bridge_entity_row(e)) : 0xdead);
        i++;
    }
    return (0);
}

BRIDGE_EXPORT(init) int init(void)
{
    bridge_term_t terms[1] = {{ 0, BRIDGE_READ, BRIDGE_AND, 0 }};

    marked = BRIDGE_COMPONENT("game:Marked", marked_t, BRIDGE_STRIP,
        BRIDGE_TABLE);
    terms[0].slot = marked;
    seeing = BRIDGE_OBSERVER("seen", BRIDGE_ON_ENTER, terms, 1, nullptr, 0, nullptr, 0);
    return (0);
}
