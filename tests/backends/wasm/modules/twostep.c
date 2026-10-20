/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "bridge_wasm.h"
#include "game.h"
#include "wasm_idle.h"

static uint32_t position;
static uint32_t doubling;
static uint32_t adding;
static uint32_t showing;

static uint32_t get_first_table(uint32_t query)
{
    return (bridge_span_table(bridge_get_span(query, 0)));
}

BRIDGE_EXPORT(twice) int twice(void)
{
    uint32_t table = get_first_table(doubling);

    BRIDGE_SET(position_t, x, position, table, 0,
        BRIDGE_GET(position_t, x, position, table, 0) * 2.0f);
    return (0);
}

BRIDGE_EXPORT(more) int more(void)
{
    uint32_t table = get_first_table(adding);

    BRIDGE_SET(position_t, x, position, table, 0,
        BRIDGE_GET(position_t, x, position, table, 0) + 1.0f);
    return (0);
}

BRIDGE_EXPORT(show) int show(void)
{
    uint32_t table = get_first_table(showing);

    BRIDGE_LOG(BRIDGE_LOG_INFO, "x",
        (uint32_t)BRIDGE_GET(position_t, x, position, table, 0), 0);
    return (0);
}

BRIDGE_EXPORT(init) int init(void)
{
    bridge_term_t terms[1] = {{ 0, BRIDGE_WRITE, BRIDGE_AND, 0 }};
    position_t p = { 1.0f, 0.0f };

    position = BRIDGE_COMPONENT("game:Position", position_t, BRIDGE_DELETE,
        BRIDGE_TABLE);
    terms[0].slot = position;
    doubling = BRIDGE_ADD_SYSTEM("twice", "update", terms, 1, nullptr, 0, nullptr, 0);
    adding = BRIDGE_ADD_SYSTEM("more", "update", terms, 1, nullptr, 0, &doubling, 1);
    terms[0].access = BRIDGE_READ;
    showing = BRIDGE_ADD_SYSTEM("show", "present", terms, 1, nullptr, 0, nullptr, 0);
    BRIDGE_ATTACH(bridge_spawn(), position, p);
    return (0);
}
