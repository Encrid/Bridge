/* A system that destroys every entity with a position, and says how many it found. Whether it
 * may is the deployment's: docs/modules/grants.md. With a read grant it has no stake in them
 * and they stay; with a write grant they go. */
#include "bridge-native.h"
#include "../game.h"

static uint32_t position, reaping;

BRIDGE_NATIVE(reap)
{
    const bridge_query *q = bridge_native_query(api, reaping);
    uint32_t rows = 0;
    for (uint32_t s = 0; s < q->spans; s ++) {
        const uint32_t *span = bridge_native_span(api, q, s, 1);
        auto who = (const uint64_t *)(api->shared_base + span[1]);
        for (uint32_t row = 0; row < span[0]; row ++) api->destroy(api, who[row]);
        rows += span[0];
    }
    bridge_native_log(api, BRIDGE_LOG_INFO, "rows", 4, rows, 0);
    return 0;
}

BRIDGE_NATIVE(init)
{
    position = BRIDGE_NATIVE_COMPONENT(api, "game:Position", Position, BRIDGE_DELETE);
    bridge_term terms[1] = {{ position, BRIDGE_READ, BRIDGE_AND, 0 }};
    reaping = BRIDGE_NATIVE_SYSTEM(api, "reap", "update", terms, 1, nullptr, 0);
    return 0;
}

BRIDGE_NATIVE(shutdown) { return 0; }
BRIDGE_NATIVE(reload) { return 0; }
