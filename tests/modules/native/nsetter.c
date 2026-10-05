/* A native module that sets a component in a system: the event a wasm observer subscribes to. */
#include "bridge-native.h"
#include "../game.h"

static const bridge_api *api;
static uint32_t position, marked, runs;
static uint64_t ids[3];

BRIDGE_NATIVE_BIND(api)

BRIDGE_NATIVE(mark)
{
    Marked m = { 77 };
    if (runs ++) return 0;
    api->attach(ids[1], marked, bridge_native_put(api, &m, sizeof m), sizeof m);
    bridge_native_log(api, BRIDGE_LOG_INFO, "marked", 6, 0, 0);
    return 0;
}

BRIDGE_NATIVE(init)
{
    int none = 0, i;
    position = BRIDGE_NATIVE_COMPONENT(api, "game:Position", Position, BRIDGE_DELETE);
    marked = BRIDGE_NATIVE_COMPONENT(api, "game:Marked", Marked, BRIDGE_STRIP);
    BRIDGE_NATIVE_SYSTEM(api, "mark", "update", &none, 0);
    for (i = 0; i < 3; i ++) {
        Position p = { (float)i, 0.0f };
        ids[i] = api->spawn();
        api->attach(ids[i], position, bridge_native_put(api, &p, sizeof p), sizeof p);
    }
    return 0;
}

BRIDGE_NATIVE(shutdown) { return 0; }
BRIDGE_NATIVE(reload) { return 0; }
