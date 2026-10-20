/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "bridge_wasm.h"
#include "user.h"
#include "wasm_idle.h"

static uint32_t body;
static uint32_t reading;

BRIDGE_EXPORT(read) int read(void)
{
    uint32_t span = bridge_get_span(reading, 0);
    uint32_t table = bridge_span_table(span);

    BRIDGE_LOG(BRIDGE_LOG_INFO, "body",
        (uint32_t)BRIDGE_GET(body_t, at.y, body, table, 0),
        (uint32_t)BRIDGE_GET(body_t, mass, body, table, 0));
    return (0);
}

BRIDGE_EXPORT(init) int init(void)
{
    bridge_term_t terms[1] = {{ 0, BRIDGE_READ, BRIDGE_AND, 0 }};
    body_t b = {{ 1.0f, 2.0f }, 3.0f };

    body = BRIDGE_COMPONENT("game:Body", body_t, BRIDGE_DELETE, BRIDGE_TABLE);
    terms[0].slot = body;
    reading = BRIDGE_ADD_SYSTEM("read", "update", terms, 1, nullptr, 0, nullptr, 0);
    BRIDGE_ATTACH(bridge_spawn(), body, b);
    return (0);
}
