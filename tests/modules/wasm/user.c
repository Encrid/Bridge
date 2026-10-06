/* Declares a component with a field of a type another module declared, and does not declare
 * that type itself. The type must outlive its declarer for as long as this is loaded. */
#include "bridge_wasm.h"

typedef struct Vec2 { float x; float y; } Vec2;
BRIDGE_SCHEMA_STRUCT(Body, { Vec2 at; float mass; });

static uint32_t body, reading;

BRIDGE_EXPORT(read) int read()
{
    uint32_t span = bridge_get_span(reading, 0, 1);
    uint32_t at = BRIDGE_ROW(Body, bridge_get_base(span, 0), 0, bridge_count_rows(span));
    BRIDGE_LOG(BRIDGE_LOG_INFO, "body", (uint32_t)BRIDGE_GET(Body, at.y, at),
               (uint32_t)BRIDGE_GET(Body, mass, at));
    return 0;
}

BRIDGE_EXPORT(init) int init()
{
    bridge_term_t terms[1] = {{ 0, BRIDGE_READ, BRIDGE_AND, 0 }};
    Body b = {{ 1.0f, 2.0f }, 3.0f };
    body = BRIDGE_COMPONENT("game:Body", Body, BRIDGE_DELETE, BRIDGE_TABLE);
    terms[0].slot = body;
    reading = BRIDGE_ADD_SYSTEM("read", "update", terms, 1, nullptr, 0);
    BRIDGE_ATTACH(bridge_spawn(), body, b);
    return 0;
}

BRIDGE_EXPORT(shutdown) int shutdown() { return 0; }
BRIDGE_EXPORT(reload) int reload() { return 0; }
