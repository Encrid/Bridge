/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "bridge_wasm.h"
#include "game.h"
#include "wasm_idle.h"

static uint32_t marked;
static uint32_t watching;
static uint32_t runs;
static uint64_t entity;

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

BRIDGE_EXPORT(any) int any(void)
{
    uint32_t count = bridge_count_triggers(watching);
    uint32_t trigger;
    uint64_t id;
    uint64_t e;
    uint32_t i = 0;

    while (i < count) {
        trigger = bridge_get_trigger(watching, i);
        id = bridge_get_trigger_id(trigger);
        e = bridge_find(id);
        BRIDGE_LOG(BRIDGE_LOG_INFO, "event",
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
    int none = 0;

    marked = BRIDGE_COMPONENT("game:Marked", marked_t, BRIDGE_STRIP,
        BRIDGE_TABLE);
    terms[0].slot = marked;
    BRIDGE_ADD_SYSTEM("poke", "update", &none, 0, &marked, 1, nullptr, 0);
    watching = BRIDGE_OBSERVER("any",
        BRIDGE_ON_ENTER | BRIDGE_ON_ATTACH | BRIDGE_ON_LEAVE, terms, 1,
        nullptr, 0, nullptr, 0);
    entity = bridge_spawn();
    return (0);
}
