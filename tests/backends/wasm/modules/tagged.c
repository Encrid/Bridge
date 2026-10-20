/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "bridge_wasm.h"
#include "game.h"
#include "wasm_idle.h"

static uint32_t position;
static uint32_t frozen;
static uint32_t counting;

BRIDGE_EXPORT(count) int count(void)
{
    uint32_t spans = bridge_count_spans(counting);
    uint32_t rows = 0;
    uint32_t carried = 0;
    uint32_t span;
    uint32_t s = 0;

    while (s < spans) {
        span = bridge_get_span(counting, s);
        rows += bridge_count_rows(span);
        carried += 1;
        s++;
    }
    BRIDGE_LOG(BRIDGE_LOG_INFO, "frozen", rows, carried);
    return (0);
}

static void populate(void)
{
    position_t p = { 0.0f, 0.0f };
    uint64_t id;
    uint32_t i = 0;

    while (i < 3) {
        id = bridge_spawn();
        BRIDGE_ATTACH(id, position, p);
        if (i)
            bridge_attach(id, frozen, BRIDGE_ZEROED);
        i++;
    }
}

BRIDGE_EXPORT(init) int init(void)
{
    bridge_term_t terms[2] = {
        { 0, BRIDGE_READ, BRIDGE_AND, 0 },
        { 0, BRIDGE_READ, BRIDGE_AND, 0 }
    };

    position = BRIDGE_COMPONENT("game:Position", position_t, BRIDGE_STRIP,
        BRIDGE_TABLE);
    frozen = BRIDGE_DECLARE_TAG("game:Frozen", BRIDGE_STRIP, BRIDGE_TABLE);
    terms[0].slot = position;
    terms[1].slot = frozen;
    counting = BRIDGE_ADD_SYSTEM("count", "update", terms, 2, nullptr, 0, nullptr, 0);
    populate();
    return (0);
}
