/* Observes game:Marked being added and overwrites it, under a write grant. Beside the watcher,
 * which reads it at the same barrier, that is a conflict only the deployment can order. */
#include "bridge-wasm.h"
#include "../game.h"

static uint32_t marked, fixing;

BRIDGE_EXPORT(fix) int fix()
{
    uint32_t count = bridge_triggers(fixing), i;
    for (i = 0; i < count; i ++) {
        uint32_t at = bridge_ref(bridge_trigger_id(bridge_trigger_at(fixing, i)), marked,
                                 sizeof(Marked));
        if (at) BRIDGE_SET(Marked, by, at, 5u);
    }
    return 0;
}

BRIDGE_EXPORT(init) int init()
{
    bridge_term terms[1] = {{ 0, BRIDGE_WRITE, BRIDGE_AND, 0 }};
    marked = BRIDGE_COMPONENT("game:Marked", Marked, BRIDGE_STRIP);
    terms[0].slot = marked;
    fixing = BRIDGE_OBSERVER("fix", BRIDGE_ON_ADD, terms, 1);
    return 0;
}

BRIDGE_EXPORT(shutdown) int shutdown() { return 0; }
BRIDGE_EXPORT(reload) int reload() { return 0; }
