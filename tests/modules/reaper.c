/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "test_module.h"
#include "game.h"
#include "idle.h"
#include "reach.h"

static uint32_t position;
static uint32_t reaping;
#ifdef MEND
static uint32_t mend;
static uint32_t runs;
#endif

static void destroy_rows(const test_api_t *api, const uint32_t *span)
{
    const uint32_t *blocks = get_blocks(api);
    const uint64_t *who = (const uint64_t *)(api->shared_base
        + blocks[span[0]]);
    uint32_t row = 0;

    while (row < test_get_rows(api, span)) {
        api->destroy(api, who[row]);
        row++;
    }
}

#ifdef MEND
static void mend_rows(const test_api_t *api, const uint32_t *span)
{
    if (runs % 2) {
        api->attach(api, api->spawn(api), mend, BRIDGE_ZEROED);
        return;
    }
    destroy_rows(api, span);
}
#endif

TEST(reap)
{
    const bridge_query_t *q = test_get_query(api, reaping);
    bridge_line_t line = { "rows", 4, 0, 0 };
    const uint32_t *span;
    uint32_t s = 0;

    while (s < q->spans) {
        span = test_get_span(api, q, s, 1);
#ifdef MEND
        mend_rows(api, span);
#else
        destroy_rows(api, span);
#endif
        line.a0 += test_get_rows(api, span);
        s++;
    }
#ifdef MEND
    runs++;
#endif
    test_log(api, BRIDGE_LOG_INFO, &line);
    return (0);
}

TEST(init)
{
    bridge_term_t terms[1] = {{ 0, BRIDGE_READ, BRIDGE_AND, 0 }};

    position = TEST_COMPONENT(api, "game:Position", position_t,
        BRIDGE_DELETE, BRIDGE_TABLE);
    terms[0].slot = position;
#ifdef MEND
    mend = TEST_DECLARE_TAG(api, "view:Mend", BRIDGE_DELETE, BRIDGE_TABLE);
    reaping = TEST_ADD_SYSTEM(api, "reap", "update", terms, 1, &mend, 1,
        nullptr, 0);
#else
    reaping = TEST_ADD_SYSTEM(api, "reap", "update", terms, 1,
        nullptr, 0, nullptr, 0);
#endif
    return (0);
}
