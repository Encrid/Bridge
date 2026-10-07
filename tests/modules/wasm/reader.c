/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge-ASM700.
**
** All rights reserved.
**
** tests/modules/wasm/reader.c -- reads game:Position under a grant, over
** the marked entities only, and says what it saw.
*/

#include "bridge_wasm.h"
#include "game.h"
#include "wasm_idle.h"

static uint32_t position;
static uint32_t marked;
static uint32_t summing;

/*
** add_span
**
** Adds to `total` the x of every position of one span, and answers how
** many rows the span has.
*/
static uint32_t add_span(uint32_t span, float *total)
{
    uint32_t count = bridge_count_rows(span);
    uint32_t base = bridge_get_base(span, 0);
    uint32_t r = 0;

    while (r < count) {
        *total += BRIDGE_GET(position_t, x,
            BRIDGE_ROW(position_t, base, r, count));
        r++;
    }
    return (count);
}

/*
** sum
**
** Says how many spans and rows its query matched, and the sum of their x.
*/
BRIDGE_EXPORT(sum) int sum(void)
{
    uint32_t spans = bridge_count_spans(summing);
    uint32_t rows = 0;
    uint32_t s = 0;
    float total = 0.0f;

    while (s < spans) {
        rows += add_span(bridge_get_span(summing, s, 2), &total);
        s++;
    }
    BRIDGE_LOG(BRIDGE_LOG_INFO, "saw", spans, rows);
    BRIDGE_LOG(BRIDGE_LOG_INFO, "sum", (uint32_t)total, 0);
    return (0);
}

/*
** init
**
** Declares the two components, and the system that reads both.
*/
BRIDGE_EXPORT(init) int init(void)
{
    bridge_term_t terms[2] = {
        { 0, BRIDGE_READ, BRIDGE_AND, 0 },
        { 0, BRIDGE_READ, BRIDGE_AND, 0 },
    };

    position = BRIDGE_COMPONENT("game:Position", position_t, BRIDGE_STRIP,
        BRIDGE_TABLE);
    marked = BRIDGE_COMPONENT("game:Marked", marked_t, BRIDGE_STRIP,
        BRIDGE_TABLE);
    terms[0].slot = position;
    terms[1].slot = marked;
    summing = BRIDGE_ADD_SYSTEM("sum", "present", terms, 2, nullptr, 0);
    return (0);
}
