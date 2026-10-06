/* A component whose fields are other declared types: a struct, an enum and a bitmask. Each is
 * declared first, in the namespace the component is in. */
#include "bridge_wasm.h"

#define VEC2_FIELDS { float x; float y; }
BRIDGE_SCHEMA_STRUCT_TYPE(Vec2, VEC2_FIELDS);
BRIDGE_SCHEMA_STRUCT_DESC(Vec2, VEC2_FIELDS);
#define TEAM_VALUES { Red, Green, Blue }
BRIDGE_SCHEMA_ENUM_TYPE(Team, uint8_t, TEAM_VALUES);
BRIDGE_SCHEMA_ENUM_DESC(Team, uint8_t, TEAM_VALUES);
#define FLAGS_VALUES { Solid = 1, Hidden = 2 }
BRIDGE_SCHEMA_BITMASK_TYPE(Flags, uint16_t, FLAGS_VALUES);
BRIDGE_SCHEMA_BITMASK_DESC(Flags, uint16_t, FLAGS_VALUES);
#define BODY_FIELDS { Vec2 at; Team team; Flags flags; float mass; }
BRIDGE_SCHEMA_STRUCT_TYPE(Body, BODY_FIELDS);
BRIDGE_SCHEMA_STRUCT_DESC(Body, BODY_FIELDS);

static uint32_t body, reading;

BRIDGE_EXPORT(read) int read()
{
    uint32_t span = bridge_get_span(reading, 0, 1);
    uint32_t at = BRIDGE_ROW(Body, bridge_get_base(span, 0), 0, bridge_count_rows(span));
    BRIDGE_LOG(BRIDGE_LOG_INFO, "body", (uint32_t)BRIDGE_GET(Body, at.y, at),
               (uint32_t)BRIDGE_GET(Body, team, at));
    BRIDGE_LOG(BRIDGE_LOG_INFO, "more", (uint32_t)BRIDGE_GET(Body, flags, at),
               (uint32_t)BRIDGE_GET(Body, mass, at));
    return 0;
}

BRIDGE_EXPORT(init) int init()
{
    bridge_term_t terms[1] = {{ 0, BRIDGE_READ, BRIDGE_AND, 0 }};
    Body b = {{ 1.0f, 2.0f }, Blue, Solid | Hidden, 4.0f };
#ifndef LEAVE_OUT_VEC2
    BRIDGE_DECLARE(BRIDGE_STRUCT, "game:Vec2", Vec2, BRIDGE_STRIP, BRIDGE_TABLE);
#endif
    BRIDGE_DECLARE(BRIDGE_ENUM, "game:Team", Team, BRIDGE_STRIP, BRIDGE_TABLE);
    BRIDGE_DECLARE(BRIDGE_BITMASK, "game:Flags", Flags, BRIDGE_STRIP, BRIDGE_TABLE);
    body = BRIDGE_COMPONENT("game:Body", Body, BRIDGE_DELETE, BRIDGE_TABLE);
    terms[0].slot = body;
    reading = BRIDGE_ADD_SYSTEM("read", "update", terms, 1, nullptr, 0);
    BRIDGE_ATTACH(bridge_spawn(), body, b);
    return 0;
}

BRIDGE_EXPORT(shutdown) int shutdown() { return 0; }
BRIDGE_EXPORT(reload) int reload() { return 0; }
