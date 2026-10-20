/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "bridge_wasm.h"
#include "game.h"
#include "wasm_idle.h"

constexpr uint32_t ENTITIES = 1000;

static uint32_t position;
static uint32_t marked;
static uint32_t moving;
static uint32_t counting;

BRIDGE_EXPORT(move) int move(void)
{
    uint32_t spans = bridge_count_spans(moving);
    uint32_t span;
    uint32_t count;
    uint32_t table;
    uint32_t s = 0;
    uint32_t r;

    while (s < spans) {
        span = bridge_get_span(moving, s);
        count = bridge_count_rows(span);
        table = bridge_span_table(span);
        r = 0;
        while (r < count) {
            BRIDGE_SET(position_t, x, position, table, r,
                BRIDGE_GET(position_t, x, position, table, r) + 1.0f);
            r++;
        }
        s++;
    }
    return (0);
}

BRIDGE_EXPORT(count) int count(void)
{
    uint32_t spans = bridge_count_spans(counting);
    uint32_t rows = 0;
    uint32_t s = 0;

    while (s < spans) {
        rows += bridge_count_rows(bridge_get_span(counting, s));
        s++;
    }
    BRIDGE_LOG(BRIDGE_LOG_INFO, "moved", spans, rows);
    return (0);
}

static void populate(void)
{
    position_t p = { 0.0f, 0.0f };
    marked_t m = {};
    uint64_t id;

    while (m.by < ENTITIES) {
        id = bridge_spawn();
        p.x = (float)m.by;
        BRIDGE_ATTACH(id, position, p);
        if (m.by % 2)
            BRIDGE_ATTACH(id, marked, m);
        m.by++;
    }
}

BRIDGE_EXPORT(init) int init(void)
{
    bridge_term_t terms[1] = {{ 0, BRIDGE_WRITE, BRIDGE_AND, 0 }};

    position = BRIDGE_COMPONENT("game:Position", position_t, BRIDGE_DELETE,
        BRIDGE_TABLE);
    marked = BRIDGE_COMPONENT("game:Marked", marked_t, BRIDGE_STRIP,
        BRIDGE_TABLE);
    terms[0].slot = position;
    moving = BRIDGE_ADD_SYSTEM("move", "update", terms, 1, nullptr, 0, nullptr, 0);
    terms[0].access = BRIDGE_READ;
    counting = BRIDGE_ADD_SYSTEM("count", "present", terms, 1, nullptr, 0, nullptr, 0);
    populate();
    return (0);
}
