/* One system never returns, and another must still run after it was stopped. */
#include "bridge_wasm.h"

static volatile int going = 1;

BRIDGE_EXPORT(forever) int forever()
{
    while (going) {}
    return 0;
}

BRIDGE_EXPORT(after) int after()
{
    BRIDGE_LOG(BRIDGE_LOG_INFO, "still here", 0, 0);
    return 0;
}

BRIDGE_EXPORT(init) int init()
{
    int none = 0;
    BRIDGE_SYSTEM("forever", "update", &none, 0, nullptr, 0);
    BRIDGE_SYSTEM("after", "present", &none, 0, nullptr, 0);
    return 0;
}

BRIDGE_EXPORT(shutdown) int shutdown() { return 0; }
BRIDGE_EXPORT(reload) int reload() { return 0; }
