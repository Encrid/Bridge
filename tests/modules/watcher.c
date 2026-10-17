/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "test_offsets.h"
#include "game.h"
#include "offsets_idle.h"

#ifdef CHANGE
constexpr uint32_t CHANGE_COUNT = 1;
#else
constexpr uint32_t CHANGE_COUNT = 0;
#endif

static uint32_t marked;
static uint32_t seeing;

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

BRIDGE_EXPORT(init) int init(void)
{
    bridge_term_t terms[1] = {{ 0, BRIDGE_READ, BRIDGE_AND, 0 }};

    marked = BRIDGE_COMPONENT("game:Marked", marked_t, BRIDGE_STRIP,
        BRIDGE_TABLE);
    terms[0].slot = marked;
    seeing = BRIDGE_OBSERVER("seen", BRIDGE_ON_ENTER, terms, 1, &marked,
        CHANGE_COUNT, nullptr, 0);
    return (0);
}
