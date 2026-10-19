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

static uint32_t get_first_row(uint32_t query)
{
    uint32_t span = bridge_get_span(query, 0, 1);
    uint32_t count = bridge_count_rows(span);

    return (BRIDGE_ROW(position_t, bridge_get_base(span, 0), 0, count));
}

BRIDGE_EXPORT(twice) int twice(void)
{
    uint32_t at = get_first_row(doubling);

    BRIDGE_SET(position_t, x, at, BRIDGE_GET(position_t, x, at) * 2.0f);
    return (0);
}

BRIDGE_EXPORT(more) int more(void)
{
    uint32_t at = get_first_row(adding);

    BRIDGE_SET(position_t, x, at, BRIDGE_GET(position_t, x, at) + 1.0f);
    return (0);
}

BRIDGE_EXPORT(show) int show(void)
{
    uint32_t at = get_first_row(showing);

    BRIDGE_LOG(BRIDGE_LOG_INFO, "x", (uint32_t)BRIDGE_GET(position_t, x, at),
        0);
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
