/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "bridge_wasm.h"
#include "hit.h"

static uint32_t add_hits(uint32_t span, uint32_t *sum)
{
    uint32_t count = bridge_count_rows(span);
    uint32_t table = bridge_span_table(span);
    uint64_t id;
    uint32_t r = 0;

    while (r < count) {
        id = bridge_get_entity(table, r);
        *sum += BRIDGE_GET(hit_t, by, hit, (uint32_t)id, 0);
        r++;
    }
    return (count);
}

BRIDGE_EXPORT(walked) int walked(void)
{
    uint32_t spans = bridge_count_spans(hits);
    uint32_t rows = 0;
    uint32_t sum = 0;
    uint32_t s = 0;

    while (s < spans) {
        rows += add_hits(bridge_get_span(hits, s), &sum);
        s++;
    }
    BRIDGE_LOG(BRIDGE_LOG_INFO, "hits", rows, sum);
    return (0);
}

static uint32_t add_reached(uint32_t span, uint32_t *found, uint32_t *sum)
{
    uint32_t count = bridge_count_rows(span);
    uint32_t table = bridge_span_table(span);
    uint64_t id;
    uint32_t r = 0;

    while (r < count) {
        id = bridge_get_entity(table, r);
        if (bridge_has(hit, id)) {
            *sum += BRIDGE_GET(hit_t, by, hit, (uint32_t)id, 0);
            (*found)++;
        }
        r++;
    }
    return (count);
}

BRIDGE_EXPORT(reached) int reached(void)
{
    uint32_t spans = bridge_count_spans(bodies);
    uint32_t rows = 0;
    uint32_t found = 0;
    uint32_t sum = 0;
    uint32_t s = 0;

    while (s < spans) {
        rows += add_reached(bridge_get_span(bodies, s), &found, &sum);
        s++;
    }
    BRIDGE_LOG(BRIDGE_LOG_INFO, "bodies", rows, found);
    BRIDGE_LOG(BRIDGE_LOG_INFO, "reached", sum, 0);
    return (0);
}
