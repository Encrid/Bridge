/* A toggled component on the wasm backend: its bits found ahead of its column, read, and
 * written. docs/bridge/storage.md, *A toggle*. */
#include "bridge_wasm.h"

BRIDGE_SCHEMA_STRUCT(Shield, { uint32_t strength; });

constexpr uint32_t ENTITIES = 10;

static uint32_t shield, lowering, counting;

/* every third one turned off: a write of its bit, by a system that stated the term as written */
BRIDGE_EXPORT(lower) int lower()
{
    uint32_t spans = bridge_spans(lowering), s, r;
    for (s = 0; s < spans; s ++) {
        uint32_t span = bridge_span(lowering, s, 1);
        uint32_t count = bridge_rows(span), base = bridge_base(span, 0), bits = bridge_bits(base);
        for (r = 0; r < count; r ++) {
            uint32_t strength = BRIDGE_GET(Shield, strength, BRIDGE_ROW(Shield, base, r, count));
            bridge_turn(bits, r, count, strength % 3 != 0);
        }
    }
    return 0;
}

BRIDGE_EXPORT(count) int count()
{
    uint32_t spans = bridge_spans(counting), rows = 0, on = 0, sum = 0, s, r;
    for (s = 0; s < spans; s ++) {
        uint32_t span = bridge_span(counting, s, 1);
        uint32_t count = bridge_rows(span), base = bridge_base(span, 0), bits = bridge_bits(base);
        for (r = 0; r < count; r ++) {
            if (!bridge_on(bits, r, count)) continue;
            sum += BRIDGE_GET(Shield, strength, BRIDGE_ROW(Shield, base, r, count));
            on ++;
        }
        rows += count;
    }
    BRIDGE_LOG(BRIDGE_LOG_INFO, "shields", rows, on);
    BRIDGE_LOG(BRIDGE_LOG_INFO, "strength", sum, 0);
    return 0;
}

BRIDGE_EXPORT(init) int init()
{
    bridge_term_t terms[1] = {{ 0, BRIDGE_WRITE, BRIDGE_AND, 0 }};
    uint32_t i;
    shield = BRIDGE_COMPONENT("game:Shield", Shield, BRIDGE_DELETE, BRIDGE_TOGGLE);
    terms[0].slot = shield;
    lowering = BRIDGE_SYSTEM("lower", "update", terms, 1, nullptr, 0);
    terms[0].access = BRIDGE_READ;
    counting = BRIDGE_SYSTEM("count", "present", terms, 1, nullptr, 0);
    for (i = 0; i < ENTITIES; i ++) {
        uint64_t id = bridge_spawn();
        Shield value = { i };
        BRIDGE_ATTACH(id, shield, value);
    }
    return 0;
}

BRIDGE_EXPORT(shutdown) int shutdown() { return 0; }
BRIDGE_EXPORT(reload) int reload() { return 0; }
