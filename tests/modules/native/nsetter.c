/* A native module that sets a component in a system: the event a wasm observer subscribes to. */
#include "bridge_native.h"
#include "../game.h"

static uint32_t position, marked, runs;
static uint64_t ids[3];


BRIDGE_NATIVE(mark)
{
    Marked m = { 77 };
    if (runs ++) return 0;
    BRIDGE_NATIVE_ATTACH(api, ids[1], marked, m);
    bridge_native_log(api, BRIDGE_LOG_INFO, &(bridge_line_t){ "marked", 6, 0, 0 });
    return 0;
}

BRIDGE_NATIVE(init)
{
    int none = 0, i;
    position = BRIDGE_NATIVE_COMPONENT(api, "game:Position", Position, BRIDGE_DELETE, BRIDGE_TABLE);
    marked = BRIDGE_NATIVE_COMPONENT(api, "game:Marked", Marked, BRIDGE_STRIP, BRIDGE_TABLE);
    BRIDGE_NATIVE_ADD_SYSTEM(api, "mark", "update", &none, 0, &marked, 1);
    for (i = 0; i < 3; i ++) {
        Position p = { (float)i, 0.0f };
        ids[i] = api->spawn(api);
        BRIDGE_NATIVE_ATTACH(api, ids[i], position, p);
    }
    return 0;
}

BRIDGE_NATIVE(shutdown) { return 0; }
BRIDGE_NATIVE(reload) { return 0; }
