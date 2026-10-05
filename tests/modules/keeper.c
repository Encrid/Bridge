/* Declares a component it shares, one only it declares, and a roster of ids so that another
 * module can ask after its entities once it is gone. Built once per on_unload policy.
 *
 * Ten entities carry Position and Private; five carry Private alone. Its shutdown destroys the
 * first of the ten: what a shutdown asks for is applied. docs/bridge/data/lifetime.md. */
#include "bridge-wasm.h"
#include "roster.h"

static uint32_t position, private_, roster;
static uint64_t first;

BRIDGE_EXPORT(init) int init(void)
{
    Roster r = {{ 0 }};
    uint32_t i;
    position = BRIDGE_COMPONENT("game:Position", Position, BRIDGE_STRIP);
    private_ = BRIDGE_COMPONENT("game:Private", Private, PRIVATE_POLICY);
    roster = BRIDGE_COMPONENT("game:Roster", Roster, BRIDGE_STRIP);
    for (i = 0; i < 15; i ++) {
        uint64_t id = bridge_spawn();
        Private secret = { i };
        BRIDGE_ATTACH(id, private_, secret);
        if (i < 10) {
            Position p = { (float)i, 0.0f };
            BRIDGE_ATTACH(id, position, p);
            if (!i) first = id;
        } else {
            r.ids[i - 10] = id;
        }
    }
    BRIDGE_ATTACH(bridge_spawn(), roster, r);
    return 0;
}

BRIDGE_EXPORT(shutdown) int shutdown(void)
{
    bridge_destroy(first);
    return 0;
}
BRIDGE_EXPORT(reload) int reload(void) { return 0; }
