/*
** Copyright (C) 2026 Bridge Owner
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/modules/user.c -- declares a component with a field of a type
** another module declared, and does not declare that type itself. The type
** must outlive its declarer for as long as this is loaded.
*/

#include "test_offsets.h"
#include "user.h"
#include "offsets_idle.h"

static uint32_t body;
static uint32_t reading;

/*
** read
**
** Says two fields of the first row: the y of the struct, and the mass.
*/
BRIDGE_EXPORT(read) int read(void)
{
    uint32_t span = bridge_get_span(reading, 0, 1);
    uint32_t at = BRIDGE_ROW(body_t, bridge_get_base(span, 0), 0,
        bridge_count_rows(span));

    BRIDGE_LOG(BRIDGE_LOG_INFO, "body", (uint32_t)BRIDGE_GET(body_t, at.y, at),
        (uint32_t)BRIDGE_GET(body_t, mass, at));
    return (0);
}

/*
** init
**
** Declares the component and the system, and spawns one entity.
*/
BRIDGE_EXPORT(init) int init(void)
{
    bridge_term_t terms[1] = {{ 0, BRIDGE_READ, BRIDGE_AND, 0 }};
    body_t b = {{ 1.0f, 2.0f }, 3.0f };

    body = BRIDGE_COMPONENT("game:Body", body_t, BRIDGE_DELETE, BRIDGE_TABLE);
    terms[0].slot = body;
    reading = BRIDGE_ADD_SYSTEM("read", "update", terms, 1, nullptr, 0);
    BRIDGE_ATTACH(bridge_spawn(), body, b);
    return (0);
}
