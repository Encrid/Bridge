/* mover.c as a shared library: it reads and writes the column through a raw pointer. */
#include "bridge-native.h"
#include "../game.h"

#define ENTITIES 1000

static const bridge_api *api;
static uint32_t position, marked, moving, tallying;

BRIDGE_NATIVE_BIND(api)

BRIDGE_NATIVE(move)
{
    const bridge_query *q = bridge_native_query(api, moving);
    uint32_t s, r;
    for (s = 0; s < q->spans; s ++) {
        const uint32_t *span = bridge_native_span(api, q, s, 1);
        Position *p = (Position *)(api->shared_base + span[2]);
        for (r = 0; r < span[0]; r ++) p[r].x += 1.0f;
    }
    return 0;
}

BRIDGE_NATIVE(tally)
{
    const bridge_query *q = bridge_native_query(api, tallying);
    uint32_t s, rows = 0;
    for (s = 0; s < q->spans; s ++) rows += bridge_native_span(api, q, s, 1)[0];
    bridge_native_log(api, 0, "native tally", 12, q->spans, rows);
    return 0;
}

BRIDGE_NATIVE(init)
{
    bridge_term terms[1] = {{ 0, BRIDGE_INOUT, BRIDGE_AND, 0 }};
    uint32_t i;
    position = BRIDGE_NATIVE_COMPONENT(api, "game:Position", Position, BRIDGE_DELETE);
    marked = BRIDGE_NATIVE_COMPONENT(api, "game:Marked", Marked, BRIDGE_STRIP);
    terms[0].slot = position;
    moving = BRIDGE_NATIVE_SYSTEM(api, "move", "update", terms, 1);
    terms[0].access = BRIDGE_IN;
    tallying = BRIDGE_NATIVE_SYSTEM(api, "tally", "present", terms, 1);
    for (i = 0; i < ENTITIES; i ++) {
        uint64_t id = api->spawn();
        Position p = { (float)i, 0.0f };
        api->attach(id, position, bridge_native_put(api, &p, sizeof p), sizeof p);
        if (i % 2) {
            Marked m = { i };
            api->attach(id, marked, bridge_native_put(api, &m, sizeof m), sizeof m);
        }
    }
    return 0;
}

BRIDGE_NATIVE(shutdown) { return 0; }
BRIDGE_NATIVE(reload) { return 0; }
