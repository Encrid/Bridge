/* Writes more lines in one phase than its ring holds. */
#include "bridge_wasm.h"

BRIDGE_EXPORT(talk) int talk()
{
    uint32_t i;
    for (i = 0; i < 100; i ++) BRIDGE_LOG(BRIDGE_LOG_INFO, "line", i, 0);
    return 0;
}

BRIDGE_EXPORT(init) int init()
{
    int none = 0;
    BRIDGE_SYSTEM("talk", "update", &none, 0, nullptr, 0);
    return 0;
}

BRIDGE_EXPORT(shutdown) int shutdown() { return 0; }
BRIDGE_EXPORT(reload) int reload() { return 0; }
