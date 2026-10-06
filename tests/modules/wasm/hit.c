/* A component kept in a sparse set, on the wasm backend: walked alone, and reached from the
 * rows of a table. docs/bridge/storage.md, *A sparse set*. */
#include "bridge_wasm.h"
#include "../game.h"

BRIDGE_SCHEMA_STRUCT(Hit, { uint32_t by; });

constexpr uint32_t ENTITIES = 10;

static uint32_t position, hit, hits, bodies;

/* the set walked: one span, as a table's is */
BRIDGE_EXPORT(walked) int walked()
{
    uint32_t spans = bridge_count_spans(hits), rows = 0, sum = 0, s, r;
    for (s = 0; s < spans; s ++) {
        uint32_t span = bridge_get_span(hits, s, 1);
        uint32_t count = bridge_count_rows(span), base = bridge_get_base(span, 0);
        for (r = 0; r < count; r ++) sum += BRIDGE_GET(Hit, by, BRIDGE_ROW(Hit, base, r, count));
        rows += count;
    }
    BRIDGE_LOG(BRIDGE_LOG_INFO, "hits", rows, sum);
    return 0;
}

/* the table walked, and Hit looked for at each row through the set's index */
BRIDGE_EXPORT(reached) int reached()
{
    uint32_t spans = bridge_count_spans(bodies), index = bridge_get_sparse_index(hit), rows = 0, found = 0, sum = 0;
    uint32_t s, r;
    for (s = 0; s < spans; s ++) {
        uint32_t span = bridge_get_span(bodies, s, 2);
        uint32_t count = bridge_count_rows(span), entities = bridge_get_entities(span);
        for (r = 0; r < count; r ++) {
            uint32_t at = bridge_get_sparse_value(index, bridge_get_entity(entities, r, count));
            if (!at) continue;
            sum += BRIDGE_GET(Hit, by, at);
            found ++;
        }
        rows += count;
    }
    BRIDGE_LOG(BRIDGE_LOG_INFO, "bodies", rows, found);
    BRIDGE_LOG(BRIDGE_LOG_INFO, "reached", sum, 0);
    return 0;
}

BRIDGE_EXPORT(init) int init()
{
    bridge_term_t one[1] = {{ 0, BRIDGE_READ, BRIDGE_AND, 0 }};
    bridge_term_t two[2] = {{ 0, BRIDGE_READ, BRIDGE_AND, 0 }, { 0, BRIDGE_READ, BRIDGE_OPTIONAL, 0 }};
    uint32_t i;
    position = BRIDGE_COMPONENT("game:Position", Position, BRIDGE_DELETE, BRIDGE_TABLE);
    hit = BRIDGE_COMPONENT("game:Hit", Hit, BRIDGE_DELETE, BRIDGE_SPARSE_SET);
    one[0].slot = hit;
    hits = BRIDGE_ADD_SYSTEM("walked", "update", one, 1, nullptr, 0);
    two[0].slot = position;
    two[1].slot = hit;
    bodies = BRIDGE_ADD_SYSTEM("reached", "present", two, 2, nullptr, 0);
    for (i = 0; i < ENTITIES; i ++) {
        uint64_t id = bridge_spawn();
        Position p = { (float)i, 0.0f };
        BRIDGE_ATTACH(id, position, p);
        if (i % 3 == 0) {
            Hit h = { i };
            BRIDGE_ATTACH(id, hit, h);
        }
    }
    return 0;
}

BRIDGE_EXPORT(shutdown) int shutdown() { return 0; }
BRIDGE_EXPORT(reload) int reload() { return 0; }
