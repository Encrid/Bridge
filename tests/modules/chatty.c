/* Writes more lines in one phase than its ring holds. */
#include "bridge-wasm.h"

BRIDGE_EXPORT(talk) int talk(void)
{
    uint32_t i;
    for (i = 0; i < 100; i ++) BRIDGE_LOG(BRIDGE_LOG_INFO, "line", i, 0);
    return 0;
}

BRIDGE_EXPORT(init) int init(void)
{
    int none = 0;
    BRIDGE_SYSTEM("talk", "update", &none, 0);
    return 0;
}

BRIDGE_EXPORT(shutdown) int shutdown(void) { return 0; }
BRIDGE_EXPORT(reload) int reload(void) { return 0; }
