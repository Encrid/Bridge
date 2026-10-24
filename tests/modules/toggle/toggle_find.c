/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "toggle.h"
#include "reach.h"

static void see_entity(const test_api_t *api, const bridge_entity_t *e,
    seen_t *seen)
{
    const bridge_header_t *h = test_get_header(api);
    const uint32_t *base = get_column_bases(api);
    uint32_t column = base[e->table * h->types + ntoggle_t];
    const uint64_t *bits;

    if (!column)
        return;
    bits = test_get_bits(api, column);
    if (!(bits[e->row >> 6] >> (e->row & 63) & 1))
        return;
    seen->total += ((const t_t *)(api->shared_base + column))[e->row].v;
    seen->on++;
}

TEST(find)
{
    const bridge_entity_t *index = get_entity_index(api);
    const bridge_entity_t *e;
    seen_t seen = {};
    uint32_t alive = 0;
    uint32_t i = 0;

    while (i < COUNT + ntoggle_more) {
        e = &index[(uint32_t)ntoggle_ids[i]];
        if (e->generation == (uint32_t)(ntoggle_ids[i] >> 32)) {
            alive++;
            see_entity(api, e, &seen);
        }
        i++;
    }
    say(api, "by id: alive, and of them on", alive, seen.on);
    say(api, "by id: the sum of those on", seen.total, 0);
    return (0);
}
