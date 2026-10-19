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
    uint32_t base = bridge_get_base(span, 0);
    uint32_t r = 0;

    while (r < count) {
        *sum += BRIDGE_GET(hit_t, by, BRIDGE_ROW(hit_t, base, r, count));
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
        rows += add_hits(bridge_get_span(hits, s, 1), &sum);
        s++;
    }
    BRIDGE_LOG(BRIDGE_LOG_INFO, "hits", rows, sum);
    return (0);
}

static uint32_t add_reached(uint32_t span, uint32_t index, uint32_t *found,
    uint32_t *sum)
{
    uint32_t count = bridge_count_rows(span);
    uint32_t entities = bridge_get_entities(span);
    uint32_t at;
    uint32_t r = 0;

    while (r < count) {
        at = bridge_get_sparse_value(index,
            bridge_get_entity(entities, r, count));
        if (at) {
            *sum += BRIDGE_GET(hit_t, by, at);
            (*found)++;
        }
        r++;
    }
    return (count);
}

BRIDGE_EXPORT(reached) int reached(void)
{
    uint32_t spans = bridge_count_spans(bodies);
    uint32_t index = bridge_get_sparse_index(hit);
    uint32_t rows = 0;
    uint32_t found = 0;
    uint32_t sum = 0;
    uint32_t s = 0;

    while (s < spans) {
        rows += add_reached(bridge_get_span(bodies, s, 2), index, &found,
            &sum);
        s++;
    }
    BRIDGE_LOG(BRIDGE_LOG_INFO, "bodies", rows, found);
    BRIDGE_LOG(BRIDGE_LOG_INFO, "reached", sum, 0);
    return (0);
}
