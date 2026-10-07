/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge-ASM700.
**
** All rights reserved.
**
** tests/modules/wasm/nested.c -- a component whose fields are other
** declared types: a struct, an enum and a bitmask. Each is declared first,
** in the namespace the component is in.
**
** Built a second way, LEAVE_OUT_VEC2: without declaring the struct one of
** its fields is.
**
** schema.md, *The type set*
*/

#include "bridge_wasm.h"
#include "nested.h"
#include "wasm_idle.h"

static uint32_t body;
static uint32_t reading;

/*
** read
**
** Says four fields of the first row: the y of the struct, the enum, the
** bitmask and the mass.
*/
BRIDGE_EXPORT(read) int read(void)
{
    uint32_t span = bridge_get_span(reading, 0, 1);
    uint32_t at = BRIDGE_ROW(body_t, bridge_get_base(span, 0), 0,
        bridge_count_rows(span));

    BRIDGE_LOG(BRIDGE_LOG_INFO, "body", (uint32_t)BRIDGE_GET(body_t, at.y, at),
        (uint32_t)BRIDGE_GET(body_t, team, at));
    BRIDGE_LOG(BRIDGE_LOG_INFO, "more",
        (uint32_t)BRIDGE_GET(body_t, flags, at),
        (uint32_t)BRIDGE_GET(body_t, mass, at));
    return (0);
}

/*
** init
**
** Declares the three types the fields are of, then the component and the
** system, and spawns one entity.
*/
BRIDGE_EXPORT(init) int init(void)
{
    bridge_term_t terms[1] = {{ 0, BRIDGE_READ, BRIDGE_AND, 0 }};
    body_t b = {{ 1.0f, 2.0f }, BLUE, SOLID | HIDDEN, 4.0f };

#ifndef LEAVE_OUT_VEC2
    BRIDGE_DECLARE(BRIDGE_STRUCT, "game:vec2_t", vec2_t, BRIDGE_STRIP,
        BRIDGE_TABLE);
#endif
    BRIDGE_DECLARE(BRIDGE_ENUM, "game:team_t", team_t, BRIDGE_STRIP,
        BRIDGE_TABLE);
    BRIDGE_DECLARE(BRIDGE_BITMASK, "game:flags_t", flags_t, BRIDGE_STRIP,
        BRIDGE_TABLE);
    body = BRIDGE_COMPONENT("game:Body", body_t, BRIDGE_DELETE, BRIDGE_TABLE);
    terms[0].slot = body;
    reading = BRIDGE_ADD_SYSTEM("read", "update", terms, 1, nullptr, 0);
    BRIDGE_ATTACH(bridge_spawn(), body, b);
    return (0);
}
