/* A direct-indexed component on the wasm backend, reached from the rows of a table by each
 * entity's id. docs/bridge/storage.md, *Direct-indexed*. */
#include "bridge-wasm.h"
#include "../game.h"

BRIDGE_SCHEMA_STRUCT(Aim, { uint32_t at; });

constexpr uint32_t ENTITIES = 10;

static uint32_t position, aim, bodies;

BRIDGE_EXPORT(reached) int reached()
{
    uint32_t spans = bridge_spans(bodies), cells = bridge_cells(aim), rows = 0, found = 0, sum = 0;
    uint32_t s, r;
    for (s = 0; s < spans; s ++) {
        uint32_t span = bridge_span(bodies, s, 2);
        uint32_t count = bridge_rows(span), entities = bridge_entities(span);
        for (r = 0; r < count; r ++) {
            uint32_t at = bridge_cell(cells, bridge_entity_at(entities, r, count), sizeof(Aim));
            if (!at) continue;
            sum += BRIDGE_GET(Aim, at, at);
            found ++;
        }
        rows += count;
    }
    BRIDGE_LOG(BRIDGE_LOG_INFO, "aims", rows, found);
    BRIDGE_LOG(BRIDGE_LOG_INFO, "aimed", sum, 0);
    return 0;
}

BRIDGE_EXPORT(init) int init()
{
    bridge_term two[2] = {{ 0, BRIDGE_READ, BRIDGE_AND, 0 }, { 0, BRIDGE_READ, BRIDGE_OPTIONAL, 0 }};
    uint32_t i;
    position = BRIDGE_COMPONENT("game:Position", Position, BRIDGE_DELETE, BRIDGE_TABLE);
    aim = BRIDGE_COMPONENT("game:Aim", Aim, BRIDGE_DELETE, BRIDGE_DIRECT_INDEXED);
    two[0].slot = position;
    two[1].slot = aim;
    bodies = BRIDGE_SYSTEM("reached", "present", two, 2, nullptr, 0);
    for (i = 0; i < ENTITIES; i ++) {
        uint64_t id = bridge_spawn();
        Position p = { (float)i, 0.0f };
        BRIDGE_ATTACH(id, position, p);
        if (i % 2 == 0) {
            Aim value = { i };
            BRIDGE_ATTACH(id, aim, value);
        }
    }
    return 0;
}

BRIDGE_EXPORT(shutdown) int shutdown() { return 0; }
BRIDGE_EXPORT(reload) int reload() { return 0; }
