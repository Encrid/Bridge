/* One observer of all three events, and a system that causes each in turn: an attach to an
 * entity without the component, an attach to one with it, and a detach. */
#include "bridge-wasm.h"
#include "game.h"

static uint32_t marked, watching, runs;
static uint64_t entity;

BRIDGE_EXPORT(poke) int poke(void)
{
    Marked m = { runs + 1 };
    if (runs < 2) BRIDGE_ATTACH(entity, marked, m);
    else if (runs == 2) bridge_detach(entity, marked);
    runs ++;
    return 0;
}

/* which events, and the value the id leads to: an id that leads nowhere was removed */
BRIDGE_EXPORT(any) int any(void)
{
    uint32_t count = bridge_triggers(watching), i;
    for (i = 0; i < count; i ++) {
        uint32_t trigger = bridge_trigger_at(watching, i);
        uint32_t at = bridge_ref(bridge_trigger_id(trigger), marked, sizeof(Marked));
        BRIDGE_LOG(BRIDGE_LOG_INFO, "event", bridge_trigger_events(trigger),
                   at ? BRIDGE_GET(Marked, by, at) : 0xdead);
    }
    return 0;
}

BRIDGE_EXPORT(init) int init(void)
{
    bridge_term terms[1] = {{ 0, BRIDGE_IN, BRIDGE_AND, 0 }};
    int none = 0;
    marked = BRIDGE_COMPONENT("game:Marked", Marked, BRIDGE_STRIP);
    terms[0].slot = marked;
    BRIDGE_SYSTEM("poke", "update", &none, 0);
    watching = BRIDGE_OBSERVER("any", BRIDGE_ON_ADD | BRIDGE_ON_SET | BRIDGE_ON_REMOVE, terms, 1);
    entity = bridge_spawn();
    return 0;
}

BRIDGE_EXPORT(shutdown) int shutdown(void) { return 0; }
