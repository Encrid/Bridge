/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "test_module.h"
#include "game.h"
#include "idle.h"

static uint32_t position;
static uint32_t reaping;

static void destroy_rows(const test_api_t *api, const uint32_t *span)
{
    const uint64_t *who = (const uint64_t *)(api->shared_base + span[1]);
    uint32_t row = 0;

    while (row < span[0]) {
        api->destroy(api, who[row]);
        row++;
    }
}

TEST(reap)
{
    const bridge_query_t *q = test_get_query(api, reaping);
    bridge_line_t line = { "rows", 4, 0, 0 };
    const uint32_t *span;
    uint32_t s = 0;

    while (s < q->spans) {
        span = test_get_span(api, q, s, 1);
        destroy_rows(api, span);
        line.a0 += span[0];
        s++;
    }
    test_log(api, BRIDGE_LOG_INFO, &line);
    return (0);
}

TEST(init)
{
    bridge_term_t terms[1] = {{ 0, BRIDGE_READ, BRIDGE_AND, 0 }};

    position = TEST_COMPONENT(api, "game:Position", position_t,
        BRIDGE_DELETE, BRIDGE_TABLE);
    terms[0].slot = position;
    reaping = TEST_ADD_SYSTEM(api, "reap", "update", terms, 1,
        nullptr, 0);
    return (0);
}
