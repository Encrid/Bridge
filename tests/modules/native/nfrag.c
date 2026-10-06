/* What it costs a system when the entities it reads are spread over many tables: one component
 * every entity carries, and markers that split them into SPLIT sets of the same size. The system
 * reads the one component of every row and says how many rows there were, so that a host can
 * time it and know the loop ran. docs/plan/STAGE10.md, *The repair*.
 *
 * Built once for each SPLIT: 1, 16, 256, 4096. The entities are the same number every time. */
#include "bridge-native.h"

BRIDGE_SCHEMA_STRUCT(V, { uint32_t v; });
#ifndef TAGS                                    /* built with TAGS, the markers are tags */
BRIDGE_SCHEMA_STRUCT(M, { uint32_t on; });
#endif

constexpr uint32_t COUNT = 204'800, MARKERS = 12;     /* 2^12 sets at the most */

static uint32_t value, markers[MARKERS], reading;


/* every entity is made with all it will carry, so each is placed once */
BRIDGE_NATIVE(fill)
{
    for (uint32_t i = 0; i < COUNT; i ++) {
        uint64_t id = api->spawn(api);
        api->attach(api, id, value, 0, 0);
        for (uint32_t k = 0; k < MARKERS; k ++) {
            if ((i % SPLIT) >> k & 1) api->attach(api, id, markers[k], 0, 0);
        }
    }
    return 0;
}

BRIDGE_NATIVE(sum)
{
    const bridge_query_t *q = bridge_native_query(api, reading);
    uint32_t rows = 0, total = 0;
    for (uint32_t s = 0; s < q->spans; s ++) {
        const uint32_t *span = bridge_native_span(api, q, s, 1);
        auto column = (const V *)(api->shared_base + span[2]);
        for (uint32_t r = 0; r < span[0]; r ++) total += column[r].v;
        rows += span[0];
    }
    /* said once a call: what was read, and over how many spans */
    bridge_native_log(api, BRIDGE_LOG_INFO, "rows, spans", 11, rows + total, q->spans);
    return 0;
}

BRIDGE_NATIVE(init)
{
    static const char *const names[MARKERS] = {
        "st:M0", "st:M1", "st:M2", "st:M3", "st:M4", "st:M5",
        "st:M6", "st:M7", "st:M8", "st:M9", "st:M10", "st:M11"
    };
    int none = 0;
    bridge_term_t one[1] = {{ 0, BRIDGE_READ, BRIDGE_AND, 0 }};
    value = BRIDGE_NATIVE_COMPONENT(api, "st:V", V, BRIDGE_DELETE, BRIDGE_TABLE);
    for (uint32_t k = 0; k < MARKERS; k ++) {
#ifdef TAGS
        markers[k] = api->declare(api, bridge_native_put(api, names[k], (uint32_t)strlen(names[k])),
            (uint32_t)strlen(names[k]), BRIDGE_TAG, 0, 1, 0, 0, BRIDGE_DELETE, BRIDGE_TABLE);
#else
        markers[k] = api->declare(api, bridge_native_put(api, names[k], (uint32_t)strlen(names[k])),
            (uint32_t)strlen(names[k]), BRIDGE_STRUCT, sizeof(M), alignof(M),
            bridge_native_put(api, M_desc, sizeof M_desc - 1), sizeof M_desc - 1, BRIDGE_DELETE, BRIDGE_TABLE);
#endif
    }
    one[0].slot = value;
    /* what `fill` attaches: the value, and every marker */
    uint32_t filled[1 + MARKERS] = { value };
    for (uint32_t k = 0; k < MARKERS; k ++) filled[1 + k] = markers[k];
    BRIDGE_NATIVE_SYSTEM(api, "fill", "spawn", &none, 0, filled, 1 + MARKERS);
    reading = BRIDGE_NATIVE_SYSTEM(api, "sum", "look", one, 1, nullptr, 0);
    return 0;
}

BRIDGE_NATIVE(shutdown) { return 0; }
BRIDGE_NATIVE(reload) { return 0; }
