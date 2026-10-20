/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "tag.h"
#include "reach.h"

static void look_index(const test_api_t *api)
{
    const uint32_t *index = test_get_sparse_index(api, ntag_p);
    uint32_t carried = 0;
    uint32_t i = 0;

    while (i < COUNT) {
        carried += index[(uint32_t)ntag_ids[i]] == BRIDGE_CARRIED;
        i++;
    }
    say(api, "P, by its index: entities whose cell says carried", carried, 0);
}

void ntag_look_set(const test_api_t *api)
{
    const bridge_query_t *q = test_get_query(api, ntag_in_set);
    const uint32_t *bases = get_column_bases(api);
    uint32_t types = test_get_header(api)->types;
    const uint32_t *span;
    uint32_t rows = 0;
    uint32_t carried = 0;
    uint32_t i = 0;

    while (i < q->spans) {
        span = test_get_span(api, q, i, 1);
        rows += span[1];
        carried += bases[span[0] * types + ntag_p] == BRIDGE_CARRIED;
        i++;
    }
    say(api, "P, in a sparse set: rows, and spans whose base says carried",
        rows, carried);
    look_index(api);
}
