/* Observes game:Marked being added, under a read grant, and says who it was added to. */
#include "bridge-wasm.h"
#include "game.h"

static uint32_t marked, seeing;

BRIDGE_EXPORT(seen) int seen(void)
{
    uint32_t count = bridge_triggers(seeing), i;
    for (i = 0; i < count; i ++) {
        uint32_t trigger = bridge_trigger_at(seeing, i);
        uint32_t at = bridge_ref(bridge_trigger_id(trigger), marked, sizeof(Marked));
        /* the value the other module attached, followed from the id */
        BRIDGE_LOG(BRIDGE_LOG_INFO, "seen", bridge_trigger_events(trigger),
                   at ? BRIDGE_GET(Marked, by, at) : 0xdead);
    }
    return 0;
}

BRIDGE_EXPORT(init) int init(void)
{
    bridge_term terms[1] = {{ 0, BRIDGE_IN, BRIDGE_AND, 0 }};
    marked = BRIDGE_COMPONENT("game:Marked", Marked, BRIDGE_STRIP);
    terms[0].slot = marked;
    seeing = BRIDGE_OBSERVER("seen", BRIDGE_ON_ADD, terms, 1);
    return 0;
}
