/* A system that attaches game:Marked, with VALUE in it, to every entity that has a position.
 * Two of these in one phase change one component of the same entities, which is what
 * docs/bridge/run/schedule.md, *Order within a phase is never implicit*, is about. Built with
 * UNSTATED, it attaches without having said so. */
#include "bridge-native.h"
#include "../game.h"

/* the phase it names: its author's word, which a deployment may call otherwise */
#ifndef PHASE
#define PHASE "update"
#endif

static uint32_t position, marked, stamping;

BRIDGE_NATIVE(stamp)
{
    const bridge_query *q = bridge_native_query(api, stamping);
    Marked m = { VALUE };
    for (uint32_t s = 0; s < q->spans; s ++) {
        const uint32_t *span = bridge_native_span(api, q, s, 1);
        auto who = (const uint64_t *)(api->shared_base + span[1]);
        for (uint32_t row = 0; row < span[0]; row ++) {
            bridge_native_attach(api, who[row], marked, &m, sizeof m);
        }
    }
    return 0;
}

BRIDGE_NATIVE(init)
{
    Position p = { 1.0f, 0.0f };
    position = BRIDGE_NATIVE_COMPONENT(api, "game:Position", Position, BRIDGE_DELETE);
    marked = BRIDGE_NATIVE_COMPONENT(api, "game:Marked", Marked, BRIDGE_STRIP);
    bridge_term terms[1] = {{ position, BRIDGE_READ, BRIDGE_AND, 0 }};
#ifdef UNSTATED
    stamping = BRIDGE_NATIVE_SYSTEM(api, "stamp", PHASE, terms, 1, nullptr, 0);
#else
    stamping = BRIDGE_NATIVE_SYSTEM(api, "stamp", PHASE, terms, 1, &marked, 1);
#endif
    bridge_native_attach(api, api->spawn(api), position, &p, sizeof p);
    return 0;
}

BRIDGE_NATIVE(shutdown) { return 0; }
BRIDGE_NATIVE(reload) { return 0; }
